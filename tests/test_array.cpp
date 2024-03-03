#include "tests_common.h"

#include "../src/array.h"

void test_array() {
	Array<char> str1 = array_new<char>(sys_allocator, (s64)(2+1));
	assert(array_add(&str1, 'h'));
	assert(array_add(&str1, 'i'));
	assert(array_add(&str1, '\0'));
	assert(!array_add(&str1, 'm'));

	printf("1 - str1: '%s'\n", str1.data);

	array_resize(&str1, 6+1);
	str1.data[str1.size-1] = ' ';
	assert(array_add(&str1, 'm'));
	assert(array_add(&str1, 'a'));
	assert(array_add(&str1, 'n'));
	assert(array_add(&str1, '\0'));
	assert(!array_add(&str1, 's'));

	printf("2 - str1: '%s'\n", str1.data);

	array_clear(&str1);
}