#include <cpp_logger.h>
#include "test_libs.h"

#ifndef LIB_CPP_LOG_LEVEL
#define LIB_CPP_LOG_LEVEL CPP_LOG_DEFAULT_LEVEL
#endif
#define LIB_CPP_LOGE(tag, ...) CPP_LOG_ERROR(LIB_CPP_LOG_LEVEL, tag, __VA_ARGS__)
#define LIB_CPP_LOGD(tag, ...) CPP_LOG_DEBUG(LIB_CPP_LOG_LEVEL, tag, __VA_ARGS__)
#define LIB_CPP_LOGT(tag, ...) CPP_LOG_TRACE(LIB_CPP_LOG_LEVEL, tag, __VA_ARGS__)

namespace
{
    const char TAG[] = "libcpp";
}

extern "C" void lib_cpp_log_all(int value)
{
    LIB_CPP_LOGE(TAG, "error %d", value);
    LIB_CPP_LOGD(TAG, "debug %d", value);
    LIB_CPP_LOGT(TAG, "trace %d", value);
}
