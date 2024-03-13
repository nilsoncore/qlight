#include "tests_common.h"

#include "../src/array.h"

struct TestStruct {
	f32 a;
	s32 b;
};

void test_array() {
	Allocator *str1_allocator = sys_allocator;
	const u32 str1_init_capacity = 5;
	Array<char> str1 = array_new<char>(str1_allocator, str1_init_capacity);
	TestAssertMessage(str1.allocator == str1_allocator, "Failed to initialize array allocator");
	TestAssertMessage(str1.capacity == str1_init_capacity, "Failed to initialize array capacity");
	TestAssertMessage(str1.size == 0, "Array size after initialization is not 0");
	TestAssertMessage(str1.data, "Failed to initialize array data");

	// Prevent TestPrint (internally `printf`) from overflow.
	memset(str1.data, 0, str1.size * sizeof(char));

	TestAssert(array_add(&str1, 'H'));
	TestAssert(array_add(&str1, 'e'));
	TestAssert(array_add(&str1, 'l'));
	TestAssert(array_add(&str1, 'l'));
	TestAssert(array_add(&str1, 'o'));
	TestAssertMessage(!array_add(&str1, ' '), "Should have failed to add to a full array");

	TestPrint("1 - Array str1: '%s'\n", str1.data);

	ArrayView<char> str1_full_view = get_array_view(&str1); // Full ArrayView
	TestAssertMessage(str1_full_view.data == str1.data, "Full ArrayView points to wrong array memory");
	TestAssertMessage(str1_full_view.size == str1.size, "Full ArrayView size is different from original array");

	TestPrint("1 - Full ArrayView str1: '%s'\n", str1_full_view.data);

	const u32 grow_size = 8;
	const u32 before_grow_size = str1.size;
	const u32 before_grow_capacity = str1.capacity;
	array_resize(&str1, str1.capacity + grow_size);
	TestAssertMessage(str1.capacity == before_grow_capacity + grow_size, "Array size is different from the one expected from resize");

	// Prevent TestPrint (internally `printf`) from overflow.
	memset(str1.data + before_grow_size, 0, grow_size);

	TestPrint("1 - Array str1: '%s'\n", str1.data);

	TestAssert(array_add(&str1, ' '));
	TestAssert(array_add(&str1, 'w'));
	TestAssert(array_add(&str1, 'o'));
	TestAssert(array_add(&str1, 'r'));
	TestAssert(array_add(&str1, 'l'));
	TestAssert(array_add(&str1, 'd'));
	TestAssert(array_add(&str1, '!'));
	TestAssert(array_add(&str1, '\n'));
	// TODO: make something like `c_string()` that return the string zero-terminated string
	// that is safe to pass to C functions; OR/AND make custom functions that support this type
	// of `string` naturally.
	TestAssertMessage(!array_add(&str1, ' '), "Should have failed to add to a full array");

	const u32 hello_offset = 0;
	const u32 hello_count = 5;
	ArrayView<char> str1_hello_view = get_array_view(&str1, 0, 5); // Partial ArrayView
	TestAssertMessage(str1_hello_view.data == str1.data + hello_offset, "Partial ArrayView points to wrong array memory");
	TestAssertMessage(str1_hello_view.size == hello_count, "Partial ArrayView size is different from given one");

	TestPrint("1 - str1: '%s'\n", str1.data);
	TestPrint("1 - Hello ArrayView str1: '%s'\n", str1.data);

	array_free(&str1);
}