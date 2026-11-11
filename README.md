# cpp_logger

Written via Claude to provide additional logging to embedded home projects

Small leveled logger for Linux, Raspberry Pi Pico, Arduino (AVR, ESP8266, ESP32) and ESP-IDF.

- Works from both C and C++ (C99 / C++11 or newer)
- No heap. Each message is formatted into a stack buffer of `CPP_LOG_BUFFER_SIZE` bytes.
- Every library sets its own compile-time level, and the project sets a default and a cap over all of them
- Disabled messages are removed by the compiler but still type checked
- One output function (sink) for the whole program, which the application can replace. The
  defaults are `printf` on Linux, Pico and ESP-IDF, and `Serial` on Arduino.
- Can be pulled in by several libraries of one project. CMake, PlatformIO, the Arduino IDE and
  ESP-IDF each end up with a single copy.

## Levels

| Value | Name                | Meaning                                |
|-------|---------------------|----------------------------------------|
| 0     | `CPP_LOG_LEVEL_NONE`  | nothing                                |
| 1     | `CPP_LOG_LEVEL_ERROR` | something failed                       |
| 2     | `CPP_LOG_LEVEL_WARN`  | something unexpected, but handled      |
| 3     | `CPP_LOG_LEVEL_INFO`  | state changes worth knowing about      |
| 4     | `CPP_LOG_LEVEL_DEBUG` | detail for debugging                   |
| 5     | `CPP_LOG_LEVEL_TRACE` | very chatty, per call or per packet    |

A message is printed only if it passes all three checks:

1. **The library's own level** (`SPARKPLUG_LOG_LEVEL`, `CPP_MQTT_LOG_LEVEL`, ...), set at compile time.
   If a library hasn't been given a level, it uses `CPP_LOG_DEFAULT_LEVEL`, which is `NONE` unless you set it.
2. **`CPP_LOG_MAX_LEVEL`**, a compile-time cap over every library (default `TRACE`).
3. **`cpp_log_set_level()`**, a run-time threshold over everything that was compiled in (default `TRACE`).

## Configuring a project

The compile-time levels are plain defines, so they can be set from any build system.

PlatformIO:

```ini
build_flags =
    -DCPP_LOG_DEFAULT_LEVEL=CPP_LOG_LEVEL_WARN   ; every library: errors and warnings
    -DCPP_MQTT_LOG_LEVEL=CPP_LOG_LEVEL_DEBUG   ; except the MQTT client, at debug
```

CMake: cpp_logger has cache variables for the project-wide settings, and each library has a
cache variable for its own level. These take a number or a name:

```sh
cmake -DCPP_LOG_DEFAULT_LEVEL=WARN -DCPP_MQTT_LOG_LEVEL=DEBUG ..
```

Replacing the sink (optional):

```cpp
#include <cpp_logger.h>

void setup()
{
    Serial1.begin(115200);
    cpp_log_set_print(Serial1);   // Arduino only: send log output to any Print

    // or anything else
    cpp_log_set_sink([](int level, const char *tag, const char *message, void *context) {
        /* write to a file, a ring buffer, MQTT, ... */
    }, nullptr);
}
```

Set the sink once at start up, before other threads or cores start logging. `cpp_log_set_sink(NULL, NULL)`
drops every message.

## Using it in a library

Add a private header that wraps the generic macros under the library's own names, so they
cannot clash with another library's:

```c
// src/MyLibLog.h. Include it from source files only, not from public headers.
#include <cpp_logger.h>

#ifndef MYLIB_LOG_LEVEL
#define MYLIB_LOG_LEVEL CPP_LOG_DEFAULT_LEVEL
#endif

#define MYLIB_LOGE(tag, ...) CPP_LOG_ERROR(MYLIB_LOG_LEVEL, tag, __VA_ARGS__)
#define MYLIB_LOGW(tag, ...) CPP_LOG_WARN(MYLIB_LOG_LEVEL, tag, __VA_ARGS__)
#define MYLIB_LOGI(tag, ...) CPP_LOG_INFO(MYLIB_LOG_LEVEL, tag, __VA_ARGS__)
#define MYLIB_LOGD(tag, ...) CPP_LOG_DEBUG(MYLIB_LOG_LEVEL, tag, __VA_ARGS__)
#define MYLIB_LOGT(tag, ...) CPP_LOG_TRACE(MYLIB_LOG_LEVEL, tag, __VA_ARGS__)
```

```c
MYLIB_LOGW("Parser", "Skipping unknown packet type %d", type);
```

Messages don't need a trailing newline; any trailing `\n` is removed before the message reaches the sink.

### CMake

```cmake
IF(NOT TARGET cpp_logger)
    include(FetchContent)
    FetchContent_Declare(cpp_logger
        GIT_REPOSITORY https://github.com/kylehofer/cpp_logger.git
        GIT_TAG main)
    FetchContent_MakeAvailable(cpp_logger)
ENDIF()

target_link_libraries(mylib PUBLIC cpp_logger)

SET(MYLIB_LOG_LEVEL "" CACHE STRING "mylib log level (NONE, ERROR, WARN, INFO, DEBUG, TRACE or 0-5)")
cpp_logger_set_level(mylib MYLIB_LOG_LEVEL "${MYLIB_LOG_LEVEL}")
```

`CPMAddPackage` works as well. The `NOT TARGET` guard means whichever library adds it first wins.
To build against a local checkout instead, pass
`-DFETCHCONTENT_SOURCE_DIR_CPP_LOGGER=/path/to/cpp_logger` (FetchContent) or
`-DCPM_cpp_logger_SOURCE=/path/to/cpp_logger` (CPM).

### PlatformIO

Add it to `library.json`; PlatformIO installs each dependency name only once:

```json
"dependencies": {
    "cpp_logger": "https://github.com/kylehofer/cpp_logger.git"
}
```

### ESP-IDF

Add the repository as a component (`components/cpp_logger`, `EXTRA_COMPONENT_DIRS`, or
`idf_component.yml`), and add `REQUIRES cpp_logger` to the components that use it.

## Options

| Define                    | Default | Meaning                                                     |
|---------------------------|---------|-------------------------------------------------------------|
| `CPP_LOG_DEFAULT_LEVEL`     | `NONE`  | Level of every library that hasn't been given one           |
| `CPP_LOG_MAX_LEVEL`         | `TRACE` | Cap over every library's level                              |
| `CPP_LOG_BUFFER_SIZE`       | `128`   | Format buffer size; longer messages end in `...`            |
| `CPP_LOG_NO_DEFAULT_SINK`   |         | Start with no sink; nothing is printed until one is set     |
| `CPP_LOG_ARDUINO_NO_SERIAL` |         | For Arduino boards without `Serial`: the default sink drops messages |

## Tests

```sh
cmake -S . -B build && cmake --build build && ctest --test-dir build
```
