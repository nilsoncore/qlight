#ifndef QLIGHT_TESTS_COMMON_H
#define QLIGHT_TESTS_COMMON_H

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../src/platform.h"

#define QLIGHT_TESTS_PRINT

#define PRINT_ALWAYS(...) printf("[" __FUNCTION__ "] " __VA_ARGS__)

#if defined(QLIGHT_TESTS_PRINT)
#define PRINT(...) PRINT_ALWAYS(__VA_ARGS__)
#else
#define PRINT(...)
#endif

#define ASSERT(expression, message) if (!(expression)) { platform_assert_fail(#expression, message, __FILE__, __LINE__); }

void test_allocator();
void test_array();

#endif /* QLIGHT_TESTS_COMMON_H */