#include "tests_common.h"

#include "../src/array.h"

void test_array() {
	Array<char> str1 = array_new<char>(sys_allocator, 5);
	assert(array_add(&str1, 'h'));
	assert(array_add(&str1, 'e'));
	assert(array_add(&str1, 'l'));
	assert(array_add(&str1, 'l'));
	assert(array_add(&str1, 'o'));
	// assert(array_add(&str1, '\0'));
	assert(!array_add(&str1, ' '));

	PRINT("1 - str1: '%s'\n", str1.data);

	// str1.data[str1.size-1] = ' ';
	array_resize(&str1, str1.size + 8);
	assert(array_add(&str1, ' '));
	assert(array_add(&str1, 'w'));
	assert(array_add(&str1, 'o'));
	assert(array_add(&str1, 'r'));
	assert(array_add(&str1, 'l'));
	assert(array_add(&str1, 'd'));
	assert(array_add(&str1, '!'));
	assert(array_add(&str1, '\n'));
	// assert(array_add(&str1, '\0'));
	assert(!array_add(&str1, ' '));

	PRINT("2 - str1: '%s'\n", str1.data);

	array_free(&str1);
}