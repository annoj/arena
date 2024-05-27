#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef void (*T_func)(void);

void T_run(const T_func func, const char *funcname, const size_t lineno);
void T_assert_is_not_null(const void *ptr);
void T_assert_char_array_equal(const char *expected, const char *actual,
                               const size_t sz);

#define T_begin() _T_begin(__FILE__)
#define T_end() _T_end()
#define T_run_test(...) T_run_test_at_line(__VA_ARGS__, __LINE__, throwaway)
#define T_run_test_at_line(func, line, ...) T_run(func, #func, line)

#ifdef TEST_IMPLEMENTATION

struct T_state {
        char *filename;
        char *funcname;
        size_t lineno;
        bool success;
        bool has_error;
} state;

void _T_begin(const char *filename)
{
        state.filename = (char *)filename;
        state.has_error = false;
}

int _T_end(void) { return state.has_error ? 1 : 0; }

void T_run(const T_func func, const char *funcname, const size_t lineno)
{
        state.funcname = (char *)funcname;
        state.lineno = lineno;
        state.success = true;

        func();

        printf("%s:%zu:%s:%s%s\n", state.filename, lineno, funcname,
               state.success ? "\033[32mOK" : "\033[31mFAIL", "\033[0m");
}

void T_assert_is_not_null(const void *ptr)
{
        if (NULL == ptr) {
                state.success = false;
                state.has_error = true;
                printf("Expected value to not be NULL, got: %p\n", (void *)ptr);
        }
}

void T_assert_char_array_equal(const char *expected, const char *actual,
                               const size_t sz)
{
        for (size_t i = 0; i == sz; i++) {
                if (expected[i] != actual[i]) {
                        state.success = false;
                        state.has_error = true;
                        printf("Expected %.*s, actual %.*s\n", (int)sz,
                               expected, (int)sz, actual);
                }
        }
}

#endif // TEST_IMPLEMENTATION
#endif // TEST_H
