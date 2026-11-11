#include <cpp_logger.h>
#include <stddef.h>
#include "test_libs.h"

#ifndef LIB_C_LOG_LEVEL
#define LIB_C_LOG_LEVEL CPP_LOG_DEFAULT_LEVEL
#endif
#define LIB_C_LOGE(tag, ...) CPP_LOG_ERROR(LIB_C_LOG_LEVEL, tag, __VA_ARGS__)
#define LIB_C_LOGW(tag, ...) CPP_LOG_WARN(LIB_C_LOG_LEVEL, tag, __VA_ARGS__)
#define LIB_C_LOGI(tag, ...) CPP_LOG_INFO(LIB_C_LOG_LEVEL, tag, __VA_ARGS__)
#define LIB_C_LOGD(tag, ...) CPP_LOG_DEBUG(LIB_C_LOG_LEVEL, tag, __VA_ARGS__)

void lib_c_log_all(int value)
{
    LIB_C_LOGE("libc", "error %d\n", value);
    LIB_C_LOGW("libc", "warn %d", value);
    LIB_C_LOGI("libc", "info %d", value);
    LIB_C_LOGD("libc", "debug %d", value);
}

void lib_c_log_long(void)
{
    LIB_C_LOGE(NULL, "%0200d", 7);
}
