#include <cpp_logger.h>
#include "test_libs.h"

/* No LIB_OFF_LOG_LEVEL given, so this library is at CPP_LOG_DEFAULT_LEVEL (NONE) */
#ifndef LIB_OFF_LOG_LEVEL
#define LIB_OFF_LOG_LEVEL CPP_LOG_DEFAULT_LEVEL
#endif

void lib_off_log_all(int value)
{
    /* `unused` is only referenced by the disabled call; -Werror fails the build if it counts as unused */
    int unused = value * 2;
    CPP_LOG_ERROR(LIB_OFF_LOG_LEVEL, "liboff", "COMPILED_OUT_MARKER error %d", unused);
    CPP_LOG_TRACE(LIB_OFF_LOG_LEVEL, "liboff", "COMPILED_OUT_MARKER trace %d", value);
}
