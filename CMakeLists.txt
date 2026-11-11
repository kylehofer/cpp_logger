cmake_minimum_required(VERSION 3.17)

# cpp_logger can be pulled in by several libraries of one project (CPM, FetchContent,
# add_subdirectory or as an ESP-IDF component); only the first one creates the target.

# ESP-IDF component
IF(COMMAND idf_component_register)
    idf_component_register(
        SRCS "src/cpp_logger.c" "src/cpp_logger_arduino.cpp"
        INCLUDE_DIRS "src"
    )
    RETURN()
ENDIF()

IF(TARGET cpp_logger)
    RETURN()
ENDIF()

IF(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    SET(CPP_LOGGER_IS_TOP_LEVEL ON)
ELSE()
    SET(CPP_LOGGER_IS_TOP_LEVEL OFF)
ENDIF()

# Always declared, as a parent may add this before its own project() call has enabled C
project(cpp_logger LANGUAGES C)

SET(CPP_LOGGER_TESTS ${CPP_LOGGER_IS_TOP_LEVEL} CACHE BOOL "Build the cpp_logger tests")

# Project wide levels. Empty keeps the defaults in cpp_logger.h. Accepts a number (0-5) or a
# name (NONE, ERROR, WARN, INFO, DEBUG, TRACE).
SET(CPP_LOG_DEFAULT_LEVEL "" CACHE STRING "Level of every library not given its own level")
SET(CPP_LOG_MAX_LEVEL "" CACHE STRING "Cap over every library's level")
SET(CPP_LOG_BUFFER_SIZE "" CACHE STRING "Per message format buffer size in bytes")

#[[
cpp_logger_level_value(<out_var> <level>)
Turns a level given as a name (DEBUG, debug, CPP_LOG_LEVEL_DEBUG) or a number into a value usable
in a compile definition.
]]
function(cpp_logger_level_value out_var level)
    string(TOUPPER "${level}" level_upper)
    IF(level_upper MATCHES "^[0-5]$")
        SET(${out_var} "${level_upper}" PARENT_SCOPE)
    ELSEIF(level_upper MATCHES "^(CPP_LOG_LEVEL_)?(NONE|ERROR|WARN|INFO|DEBUG|TRACE)$")
        SET(${out_var} "CPP_LOG_LEVEL_${CMAKE_MATCH_2}" PARENT_SCOPE)
    ELSE()
        message(FATAL_ERROR "cpp_logger: '${level}' is not a log level (0-5, NONE, ERROR, WARN, INFO, DEBUG or TRACE)")
    ENDIF()
endfunction()

#[[
cpp_logger_set_level(<target> <macro> <level>)
Sets a library's own compile-time level, e.g.
    cpp_logger_set_level(cpp_sparkplug SPARKPLUG_LOG_LEVEL "${CPP_SPARKPLUG_LOG_LEVEL}")
Does nothing when <level> is empty, so the library falls back to CPP_LOG_DEFAULT_LEVEL.
The definition is PRIVATE: libraries only log from their own source files.
]]
function(cpp_logger_set_level target macro level)
    IF(NOT "${level}" STREQUAL "")
        cpp_logger_level_value(value "${level}")
        target_compile_definitions(${target} PRIVATE "${macro}=${value}")
    ENDIF()
endfunction()

add_library(cpp_logger STATIC src/cpp_logger.c)
add_library(cpp_logger::cpp_logger ALIAS cpp_logger)

target_include_directories(cpp_logger PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

# May end up linked into a shared library
set_target_properties(cpp_logger PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    C_STANDARD 99
)

# PUBLIC so every library and the application see the same project wide levels
IF(NOT "${CPP_LOG_DEFAULT_LEVEL}" STREQUAL "")
    cpp_logger_level_value(CPP_LOGGER_VALUE "${CPP_LOG_DEFAULT_LEVEL}")
    target_compile_definitions(cpp_logger PUBLIC "CPP_LOG_DEFAULT_LEVEL=${CPP_LOGGER_VALUE}")
ENDIF()

IF(NOT "${CPP_LOG_MAX_LEVEL}" STREQUAL "")
    cpp_logger_level_value(CPP_LOGGER_VALUE "${CPP_LOG_MAX_LEVEL}")
    target_compile_definitions(cpp_logger PUBLIC "CPP_LOG_MAX_LEVEL=${CPP_LOGGER_VALUE}")
ENDIF()

IF(NOT "${CPP_LOG_BUFFER_SIZE}" STREQUAL "")
    target_compile_definitions(cpp_logger PRIVATE "CPP_LOG_BUFFER_SIZE=${CPP_LOG_BUFFER_SIZE}")
ENDIF()

# No pico_stdlib link is needed: the Pico application's own link wraps printf for every object,
# and the Pico toolchain file already sets the CPU flags.

IF(CPP_LOGGER_TESTS AND NOT CMAKE_CROSSCOMPILING)
    enable_testing()
    add_subdirectory(tests)
ENDIF()
