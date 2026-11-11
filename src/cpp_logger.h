/*
 * File: cpp_logger.h
 * Project: cpp_logger
 *
 * MIT License
 *
 * Copyright (c) 2026 Kyle Hofer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CPP_LOGGER_H
#define CPP_LOGGER_H

/**
 * Small leveled logger for Linux, Raspberry Pi Pico, Arduino (AVR, ESP8266, ESP32) and ESP-IDF.
 * Usable from C and C++ (C99 / C++11 or newer). No heap; each message is formatted into a stack
 * buffer of CPP_LOG_BUFFER_SIZE bytes and handed to a single sink.
 *
 * Levels are decided at three points, and a message has to pass all of them:
 *
 *   1. The calling library's own compile-time level. Every library that logs picks its own macro
 *      (SPARKPLUG_LOG_LEVEL, CPP_MQTT_LOG_LEVEL, ...) that defaults to CPP_LOG_DEFAULT_LEVEL, so a
 *      project can set one library to DEBUG and leave the others quiet.
 *   2. CPP_LOG_MAX_LEVEL, a compile-time cap over every library.
 *   3. cpp_log_set_level(), a run-time threshold over everything that was compiled in.
 *
 * Messages above the compile-time levels are removed by the compiler: their format strings and
 * arguments do not end up in the binary, but they are still type checked.
 *
 * Library usage: wrap the generic macros once in a private header, so the library's macros cannot
 * clash with another library's:
 *
 *     #include <cpp_logger.h>
 *     #ifndef MYLIB_LOG_LEVEL
 *     #define MYLIB_LOG_LEVEL CPP_LOG_DEFAULT_LEVEL
 *     #endif
 *     #define MYLIB_LOGE(tag, ...) CPP_LOG_ERROR(MYLIB_LOG_LEVEL, tag, __VA_ARGS__)
 *     ...
 *
 * Application usage: set the levels with -D flags, and optionally replace the sink at start up.
 */

#include <stdarg.h>

#define CPP_LOG_LEVEL_NONE 0
#define CPP_LOG_LEVEL_ERROR 1
#define CPP_LOG_LEVEL_WARN 2
#define CPP_LOG_LEVEL_INFO 3
#define CPP_LOG_LEVEL_DEBUG 4
#define CPP_LOG_LEVEL_TRACE 5

/**
 * Compile-time level used by every library that has not been given a level of its own.
 * Off by default, so adding the logger to a library does not change its output or size.
 */
#ifndef CPP_LOG_DEFAULT_LEVEL
#define CPP_LOG_DEFAULT_LEVEL CPP_LOG_LEVEL_NONE
#endif

/** Compile-time cap over every library's level. */
#ifndef CPP_LOG_MAX_LEVEL
#define CPP_LOG_MAX_LEVEL CPP_LOG_LEVEL_TRACE
#endif

/**
 * Size of the stack buffer each message is formatted into, including the terminating NUL.
 * Longer messages are truncated and end in "...". Only read by cpp_logger.c.
 */
#ifndef CPP_LOG_BUFFER_SIZE
#define CPP_LOG_BUFFER_SIZE 128
#endif

#if defined(__GNUC__) || defined(__clang__)
#define CPP_LOG_PRINTF_FORMAT(format_index, first_arg) __attribute__((format(printf, format_index, first_arg)))
#else
#define CPP_LOG_PRINTF_FORMAT(format_index, first_arg)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Receives every message that passes the level checks.
     *
     * @param level   CPP_LOG_LEVEL_ERROR .. CPP_LOG_LEVEL_TRACE
     * @param tag     Name of the logging component, may be NULL
     * @param message Formatted message, NUL terminated, without a trailing newline
     * @param context The pointer given to cpp_log_set_sink()
     */
    typedef void (*cpp_log_sink_t)(int level, const char *tag, const char *message, void *context);

    /**
     * @brief Replaces the sink. NULL discards every message.
     * Not synchronised: set it once at start up, before other threads or cores log.
     */
    void cpp_log_set_sink(cpp_log_sink_t sink, void *context);

    /** @brief The current sink, or NULL. */
    cpp_log_sink_t cpp_log_get_sink(void);

    /**
     * @brief Sink used until cpp_log_set_sink() is called. Writes "[E] tag: message" lines with
     * printf on Linux, Pico and ESP-IDF, and to Serial on Arduino.
     */
    void cpp_log_default_sink(int level, const char *tag, const char *message, void *context);

    /** @brief Run-time threshold; messages above it are dropped. Defaults to CPP_LOG_LEVEL_TRACE. */
    void cpp_log_set_level(int level);
    int cpp_log_get_level(void);

    /** @brief "ERROR", "WARN", "INFO", "DEBUG", "TRACE", or "?" */
    const char *cpp_log_level_name(int level);

    /** @brief 'E', 'W', 'I', 'D', 'T', or '?' */
    char cpp_log_level_letter(int level);

    /**
     * @brief Formats and writes a message, subject only to the run-time level.
     * Prefer the macros below, which also apply the compile-time levels.
     */
    void cpp_log_write(int level, const char *tag, const char *format, ...) CPP_LOG_PRINTF_FORMAT(3, 4);
    void cpp_log_vwrite(int level, const char *tag, const char *format, va_list args);

#ifdef __cplusplus
}
#endif

/** True when a message of `level` from a library compiled at `module_level` is compiled in. */
#define CPP_LOG_ENABLED(module_level, level) \
    ((level) > CPP_LOG_LEVEL_NONE && (level) <= (module_level) && (level) <= CPP_LOG_MAX_LEVEL)

/**
 * The condition is a compile-time constant, so a disabled call is dropped by the compiler while
 * its arguments are still checked against the format and count as used.
 */
#define CPP_LOG_AT(module_level, level, tag, ...)          \
    do                                                    \
    {                                                     \
        if (CPP_LOG_ENABLED(module_level, level))           \
        {                                                 \
            cpp_log_write((level), (tag), __VA_ARGS__);     \
        }                                                 \
    } while (0)

#define CPP_LOG_ERROR(module_level, tag, ...) CPP_LOG_AT(module_level, CPP_LOG_LEVEL_ERROR, tag, __VA_ARGS__)
#define CPP_LOG_WARN(module_level, tag, ...) CPP_LOG_AT(module_level, CPP_LOG_LEVEL_WARN, tag, __VA_ARGS__)
#define CPP_LOG_INFO(module_level, tag, ...) CPP_LOG_AT(module_level, CPP_LOG_LEVEL_INFO, tag, __VA_ARGS__)
#define CPP_LOG_DEBUG(module_level, tag, ...) CPP_LOG_AT(module_level, CPP_LOG_LEVEL_DEBUG, tag, __VA_ARGS__)
#define CPP_LOG_TRACE(module_level, tag, ...) CPP_LOG_AT(module_level, CPP_LOG_LEVEL_TRACE, tag, __VA_ARGS__)

#if defined(ARDUINO) && defined(__cplusplus)
class Print;

/** @brief Arduino: send log lines to any Print, e.g. cpp_log_set_print(Serial1). */
void cpp_log_set_print(Print &output);
#endif

#endif /* CPP_LOGGER_H */
