# Fails if a message from a library built at CPP_LOG_LEVEL_NONE made it into the binary
file(STRINGS "${FILE}" found REGEX "COMPILED_OUT_MARKER")
IF(found)
    message(FATAL_ERROR "Disabled log messages were compiled in: ${found}")
ENDIF()
