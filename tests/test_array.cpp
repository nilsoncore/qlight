#include "tests_common.h"

#include "../src/array.h"

struct TestStruct {
	f32 a;
	s32 b;
};

void test_array() {
	Array<char> str1 = array_new<char>(sys_allocator, 5);
	TestAssert(array_add(&str1, 'h'));
	TestAssert(array_add(&str1, 'e'));
	TestAssert(array_add(&str1, 'l'));
	TestAssert(array_add(&str1, 'l'));
	TestAssert(array_add(&str1, 'o'));
	// TestAssert(array_add(&str1, '\0'));
	TestAssertMessage(!array_add(&str1, ' '), "Should have failed to add to a full array.");

	TestPrint("1 - str1: '%s'\n", str1.data);

	// str1.data[str1.size-1] = ' ';
	array_resize(&str1, str1.size + 8);
	TestAssert(array_add(&str1, ' '));
	TestAssert(array_add(&str1, 'w'));
	TestAssert(array_add(&str1, 'o'));
	TestAssert(array_add(&str1, 'r'));
	TestAssert(array_add(&str1, 'l'));
	TestAssert(array_add(&str1, 'd'));
	TestAssert(array_add(&str1, '!'));
	TestAssert(array_add(&str1, '\n'));
	// TestAssert(array_add(&str1, '\0'));
	TestAssertMessage(!array_add(&str1, ' '), "Should have failed to add to a full array.");

	TestPrint("2 - str1: '%s'\n", str1.data);

	array_free(&str1);
}