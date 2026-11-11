/*
 * File: cpp_logger.c
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

#include "cpp_logger.h"

#include <stdio.h>
#include <string.h>

#if CPP_LOG_BUFFER_SIZE < 8
#error "CPP_LOG_BUFFER_SIZE must be at least 8"
#endif

#if defined(ARDUINO)
// Defined in cpp_logger_arduino.cpp, as Serial is only reachable from C++
void cpp_log_arduino_print_sink(int level, const char *tag, const char *message, void *context);
#endif

static cpp_log_sink_t cpp_log_sink =
#if defined(CPP_LOG_NO_DEFAULT_SINK)
    NULL;
#else
    cpp_log_default_sink;
#endif

static void *cpp_log_sink_context = NULL;
static volatile int cpp_log_level = CPP_LOG_LEVEL_TRACE;

void cpp_log_set_sink(cpp_log_sink_t sink, void *context)
{
    cpp_log_sink_context = context;
    cpp_log_sink = sink;
}

cpp_log_sink_t cpp_log_get_sink(void)
{
    return cpp_log_sink;
}

void cpp_log_set_level(int level)
{
    cpp_log_level = level;
}

int cpp_log_get_level(void)
{
    return cpp_log_level;
}

const char *cpp_log_level_name(int level)
{
    switch (level)
    {
    case CPP_LOG_LEVEL_ERROR:
        return "ERROR";
    case CPP_LOG_LEVEL_WARN:
        return "WARN";
    case CPP_LOG_LEVEL_INFO:
        return "INFO";
    case CPP_LOG_LEVEL_DEBUG:
        return "DEBUG";
    case CPP_LOG_LEVEL_TRACE:
        return "TRACE";
    default:
        return "?";
    }
}

char cpp_log_level_letter(int level)
{
    return cpp_log_level_name(level)[0];
}

void cpp_log_default_sink(int level, const char *tag, const char *message, void *context)
{
#if defined(ARDUINO)
    cpp_log_arduino_print_sink(level, tag, message, context);
#else
    (void)context;
    if (tag != NULL && tag[0] != '\0')
    {
        printf("[%c] %s: %s\n", cpp_log_level_letter(level), tag, message);
    }
    else
    {
        printf("[%c] %s\n", cpp_log_level_letter(level), message);
    }
#endif
}

void cpp_log_vwrite(int level, const char *tag, const char *format, va_list args)
{
    cpp_log_sink_t sink = cpp_log_sink;
    void *context = cpp_log_sink_context;
    char buffer[CPP_LOG_BUFFER_SIZE];
    size_t length;
    int written;

    if (sink == NULL || level <= CPP_LOG_LEVEL_NONE || level > cpp_log_level || format == NULL)
    {
        return;
    }

    written = vsnprintf(buffer, sizeof(buffer), format, args);

    if (written < 0)
    {
        return;
    }

    if ((size_t)written >= sizeof(buffer))
    {
        // Truncated: mark it so a cut-off message is not mistaken for a whole one
        memcpy(buffer + sizeof(buffer) - 4, "...", 4);
    }
    else
    {
        // Sinks end the line themselves; this lets callers keep their old "\n" terminated formats
        length = (size_t)written;
        while (length > 0 && (buffer[length - 1] == '\n' || buffer[length - 1] == '\r'))
        {
            buffer[--length] = '\0';
        }
    }

    sink(level, tag, buffer, context);
}

void cpp_log_write(int level, const char *tag, const char *format, ...)
{
    va_list args;

    // Checked here as well so a filtered message skips va_start and the formatting
    if (cpp_log_sink == NULL || level <= CPP_LOG_LEVEL_NONE || level > cpp_log_level)
    {
        return;
    }

    va_start(args, format);
    cpp_log_vwrite(level, tag, format, args);
    va_end(args);
}
