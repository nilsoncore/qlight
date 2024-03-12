#ifndef QLIGHT_TESTS_COMMON_H
#define QLIGHT_TESTS_COMMON_H

#include <stdio.h>
#include <string.h>

#include "../src/platform.h"

#define QLIGHT_TESTS_PRINT

#define TestPrintAlways(...) printf("[" __FUNCTION__ "] " __VA_ARGS__)

#if defined(QLIGHT_TESTS_PRINT)
	#define TestPrint(...) TestPrintAlways(__VA_ARGS__)
#else
	#define TestPrint(...)
#endif

// Always assert with message.
#define TestAssertMessage(expression, message) \
	if (!(expression)) { platform_assert_fail(#expression, message, __FILE__, __LINE__); }

// Always assert.
#define TestAssert(expression) TestAssertMessage(expression, "-")

void test_allocator();
void test_array();

#endif /* QLIGHT_TESTS_COMMON_H */