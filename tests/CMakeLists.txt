enable_language(CXX)

# Two stand-in libraries at different levels, one in C and one in C++, sharing one sink
add_library(cpp_log_test_lib_c STATIC lib_c.c)
target_link_libraries(cpp_log_test_lib_c PUBLIC cpp_logger)
cpp_logger_set_level(cpp_log_test_lib_c LIB_C_LOG_LEVEL WARN)

add_library(cpp_log_test_lib_cpp STATIC lib_cpp.cpp)
target_link_libraries(cpp_log_test_lib_cpp PUBLIC cpp_logger)
cpp_logger_set_level(cpp_log_test_lib_cpp LIB_CPP_LOG_LEVEL debug)
set_target_properties(cpp_log_test_lib_cpp PROPERTIES CXX_STANDARD 11)

# Compiled with no level set and without optimisation; none of its strings may reach the binary
add_library(cpp_log_test_lib_off STATIC lib_off.c)
target_link_libraries(cpp_log_test_lib_off PUBLIC cpp_logger)
target_compile_options(cpp_log_test_lib_off PRIVATE -O0 -Wall -Wextra -Werror)

add_executable(cpp_logger_tests tests.c)
target_link_libraries(cpp_logger_tests cpp_log_test_lib_c cpp_log_test_lib_cpp cpp_log_test_lib_off)
target_compile_options(cpp_log_test_lib_c PRIVATE -Wall -Wextra -Werror)
target_compile_options(cpp_log_test_lib_cpp PRIVATE -Wall -Wextra -Werror)
target_compile_options(cpp_logger_tests PRIVATE -Wall -Wextra -Werror)
target_compile_options(cpp_logger PRIVATE -Wall -Wextra -Werror)

add_test(NAME cpp_logger_tests COMMAND cpp_logger_tests)
add_test(NAME cpp_logger_compiled_out
    COMMAND ${CMAKE_COMMAND} -DFILE=$<TARGET_FILE:cpp_logger_tests> -P ${CMAKE_CURRENT_SOURCE_DIR}/check_compiled_out.cmake)
