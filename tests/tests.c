#include <cpp_logger.h>
#include <stdio.h>
#include <string.h>
#include "test_libs.h"

#define MAX_LINES 16

static char lines[MAX_LINES][CPP_LOG_BUFFER_SIZE + 32];
static int line_count;
static int failures;
static void *last_context;

static void capture_sink(int level, const char *tag, const char *message, void *context)
{
    last_context = context;
    if (line_count < MAX_LINES)
    {
        snprintf(lines[line_count++], sizeof(lines[0]), "%c|%s|%s", cpp_log_level_letter(level), tag ? tag : "", message);
    }
}

static void reset(void)
{
    line_count = 0;
    memset(lines, 0, sizeof(lines));
}

#define CHECK(condition)                                                    \
    do                                                                      \
    {                                                                       \
        if (!(condition))                                                   \
        {                                                                   \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);     \
            failures++;                                                     \
        }                                                                   \
    } while (0)

#define CHECK_LINE(index, expected) CHECK(line_count > (index) && strcmp(lines[index], expected) == 0)

static void test_per_library_levels(void)
{
    reset();
    lib_c_log_all(1);   /* WARN */
    lib_cpp_log_all(2); /* DEBUG */
    lib_off_log_all(3); /* NONE */

    CHECK(line_count == 4);
    CHECK_LINE(0, "E|libc|error 1"); /* trailing newline stripped */
    CHECK_LINE(1, "W|libc|warn 1");
    CHECK_LINE(2, "E|libcpp|error 2");
    CHECK_LINE(3, "D|libcpp|debug 2");
    CHECK(last_context == &line_count);
}

static void test_runtime_level(void)
{
    reset();
    cpp_log_set_level(CPP_LOG_LEVEL_ERROR);
    CHECK(cpp_log_get_level() == CPP_LOG_LEVEL_ERROR);
    lib_c_log_all(1);
    lib_cpp_log_all(2);
    CHECK(line_count == 2);
    CHECK_LINE(0, "E|libc|error 1");
    CHECK_LINE(1, "E|libcpp|error 2");

    reset();
    cpp_log_set_level(CPP_LOG_LEVEL_NONE);
    lib_c_log_all(1);
    CHECK(line_count == 0);
    cpp_log_set_level(CPP_LOG_LEVEL_TRACE);
}

static void test_truncation(void)
{
    size_t length;

    reset();
    lib_c_log_long();
    CHECK(line_count == 1);
    length = strlen(lines[0]);
    /* "E||" + CPP_LOG_BUFFER_SIZE - 1 characters, the last three being "..." */
    CHECK(length == 3 + CPP_LOG_BUFFER_SIZE - 1);
    CHECK(length > 3 && strcmp(lines[0] + length - 3, "...") == 0);
}

static void test_direct_write_and_bad_levels(void)
{
    reset();
    cpp_log_write(CPP_LOG_LEVEL_INFO, "app", "value=%s", "x");
    cpp_log_write(CPP_LOG_LEVEL_NONE, "app", "never");
    cpp_log_write(42, "app", "never");
    cpp_log_write(-1, "app", "never");
    CHECK(line_count == 1);
    CHECK_LINE(0, "I|app|value=x");
}

static void test_null_sink(void)
{
    reset();
    cpp_log_set_sink(NULL, NULL);
    CHECK(cpp_log_get_sink() == NULL);
    lib_c_log_all(1);
    cpp_log_write(CPP_LOG_LEVEL_ERROR, "app", "dropped");
    CHECK(line_count == 0);
}

static void test_names(void)
{
    CHECK(strcmp(cpp_log_level_name(CPP_LOG_LEVEL_WARN), "WARN") == 0);
    CHECK(cpp_log_level_letter(CPP_LOG_LEVEL_TRACE) == 'T');
    CHECK(strcmp(cpp_log_level_name(9), "?") == 0);
}

int main(void)
{
    CHECK(cpp_log_get_sink() == cpp_log_default_sink);
    cpp_log_write(CPP_LOG_LEVEL_INFO, "tests", "default sink output");
    cpp_log_write(CPP_LOG_LEVEL_INFO, NULL, "default sink output without a tag");

    cpp_log_set_sink(capture_sink, &line_count);

    test_per_library_levels();
    test_runtime_level();
    test_truncation();
    test_direct_write_and_bad_levels();
    test_null_sink();
    test_names();

    printf(failures ? "%d failure(s)\n" : "All tests passed\n", failures);
    return failures ? 1 : 0;
}
