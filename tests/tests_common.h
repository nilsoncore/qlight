#ifndef QLIGHT_TESTS_COMMON_H
#define QLIGHT_TESTS_COMMON_H

#include <stdio.h>
#include <string.h>

#include "../src/platform.h"

/*
	Treat warning as error.
	C4002: too many actual parameters for macro 'identifier'.
	Example:
		// Here, `TestAssert(expression)` is used by mistake instead of `TestAssertMessage(expression, message)`.
		TestAssert(str1.size == 0, "Failed to reset array size on clear");
*/
#pragma warning( error : 4002 ) // C4002: too many actual parameters for macro 'identifier'.

#define QLIGHT_TESTS_PRINT

#define TestAlwaysPrintPrefixed(prefix, ...)  printf(prefix __VA_ARGS__);
#define TestAlwaysPrint(...)                  TestAlwaysPrintPrefixed("[" __FUNCTION__ "] ", __VA_ARGS__)
#define TestAlwaysWrite(buffer, count)        fwrite(buffer, sizeof(char), count, stdout)

#define TestAlwaysWritePrefixed(buffer, count) \
	const char *prefix = "[" __FUNCTION__ "] "; \
	TestAlwaysWrite(prefix, sizeof(prefix) - 1); \
	TestAlwaysWrite(buffer, count)

#if defined(QLIGHT_TESTS_PRINT)
	#define TestPrintPrefixed(prefix, ...)    TestAlwaysPrintPrefixed(prefix, __VA_ARGS__)
	#define TestPrint(...)                    TestAlwaysPrint(__VA_ARGS__)
	#define TestWrite(buffer, count)          TestAlwaysWrite(buffer, count)
	#define TestWritePrefixed(buffer, count)  TestAlwaysWritePrefixed(buffer, count)
#else
	#define TestPrintPrefixed(prefix, ...)
	#define TestPrint(...)
	#define TestWrite(buffer, count)
	#define TestWritePrefixed(buffer, count)
#endif

// Always assert with message.
#define TestAssertMessage(expression, message) \
	if (!(expression)) { platform_assert_fail(#expression, message, __FILE__, __LINE__); }

// Always assert.
#define TestAssert(expression) TestAssertMessage(expression, "-")

void test_allocator();
void test_array();

#endif /* QLIGHT_TESTS_COMMON_H */