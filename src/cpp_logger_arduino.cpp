/*
 * File: cpp_logger_arduino.cpp
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

// Arduino output. Compiles to nothing on other platforms, so build systems can always include it.
#if defined(ARDUINO)

#include <Arduino.h>
#include "cpp_logger.h"

extern "C" void cpp_log_arduino_print_sink(int level, const char *tag, const char *message, void *context)
{
    Print *output = static_cast<Print *>(context);

    if (output == nullptr)
    {
#if defined(CPP_LOG_ARDUINO_NO_SERIAL)
        return;
#else
        output = &Serial;
#endif
    }

    output->print('[');
    output->print(cpp_log_level_letter(level));
    output->print("] ");

    if (tag != nullptr && tag[0] != '\0')
    {
        output->print(tag);
        output->print(": ");
    }

    output->println(message);
}

void cpp_log_set_print(Print &output)
{
    cpp_log_set_sink(cpp_log_arduino_print_sink, &output);
}

#endif
