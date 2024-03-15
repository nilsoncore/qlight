#include "tests_common.h"

#include "../src/array.h"

void test_array() {

	TestPrintPrefixed("", "\n  --- Array Initialization: array_new() ---\n\n");

	TestAssert(false);
	Allocator *str1_allocator = sys_allocator;

	const char c_hello[] = "Hello";
	const size_t c_hello_size_minus_1 = sizeof(c_hello) - 1;

	const u32 str1_init_capacity = c_hello_size_minus_1;
	const u32 str1_init_size = 0;

	TestPrint("Creating new array str1... (allocator: %p, capacity: %u)\n", str1_allocator, str1_init_capacity);

	Array<char> str1 = array_new<char>(str1_allocator, str1_init_capacity);

	TestPrint("str1.allocator: %p (expected: %p)\n", str1.allocator, str1_allocator);
	TestAssert(str1.allocator == str1_allocator);

	TestPrint("str1.capacity: %u (expected: %u)\n", str1.capacity, str1_init_capacity);
	TestAssert(str1.capacity == str1_init_capacity);

	TestPrint("str1.size: %u (expected: %u)\n", str1.size, str1_init_size);
	TestAssert(str1.size == 0);

	TestPrint("str1.data: %p (expected: not NULL)\n", str1.data);
	TestAssert(str1.data);

	// TODO(nilsoncore):
	// This ugly code exists as it is because there is no
	// support for custom strings like Array<char> or String yet.
	//
	// NOTE(nilsoncore):
	// C functions like `printf()`, `fputs()`, and others expect that a
	// null terminated string is passed, but the custom strings have field a `size`
	// packed along with them so they don't get constantly checked for
	// a null terminator character.  If a custom string passed to a C function, then
	// the function will print its contents and whatever garbage it is followed by
	// in the memory until it hits a zero character.  Of course, this is totally
	// wrong and not safe. Custom strings are printed through the `fwrite()`
	// function for now, which can print the exact number of bytes you pass it
	//  in arguments, which is great.
	//
	// Alternatively, the {fmt} library formatting could be used with a simple call like:
	//     fmt::print("str1.data: {:.{}}\n", str.data, str.size);
	//
	// At first sight {fmt} has a reasonable printing format and is simple to use,
	// but the library itself is full of C++ craziness and is unreadable for a human
	// whatsoever, so probably custom functions would be made in the future.
	TestPrint("str1.data: \"");
	TestWrite(str1.data, str1.size);
	TestPrintPrefixed("", "\" (expected: \"\")\n");


	TestPrintPrefixed("", "\n  --- Array Appending: array_add() ---\n\n");


	for (u32 char_idx = 0; char_idx < c_hello_size_minus_1; char_idx += 1) {
		const char current_char = c_hello[char_idx];
		TestPrint("Adding char '%c' to str1...\n", current_char);
		TestAssert(array_add(&str1, current_char));
	}

	TestPrint("str1.data: \"");
	TestWrite(str1.data, str1.size);
	TestPrintPrefixed("", "\" (expected: \"%s\")\n", c_hello);

	const char test_char = '!';
	TestPrint("Adding char '%c' to str1... (It must not be added since array is full)\n", test_char);
	TestAssert(!array_add(&str1, test_char));

	TestPrint("str1.data: \"");
	TestWrite(str1.data, str1.size);
	TestPrintPrefixed("", "\" (expected: \"%s\")\n", c_hello);


	TestPrintPrefixed("", "\n  --- Array View: get_array_view() ---\n\n");


	TestPrint("Creating new ArrayView of str1... (array: %p)\n", &str1);

	ArrayView<char> str1_view_full = get_array_view(&str1); // Full ArrayView

	TestPrint("str1_view_full.data: %p (expected: %p)\n", str1_view_full.data, str1.data);
	TestAssert(str1_view_full.data == str1.data);

	TestPrint("str1_view_full.size: %u (expected: %u)\n", str1_view_full.size, str1.size);
	TestAssert(str1_view_full.size == str1.size);

	TestPrint("str1_view_full.data: \"");
	TestWrite(str1_view_full.data, str1_view_full.size);
	TestPrintPrefixed("", "\" (expected: \"");
	TestWrite(str1.data, str1.size);
	TestPrintPrefixed("", "\")\n\n");

	Allocator *str2_allocator = sys_allocator;

	const char c_running_in_circles[] = "Running in circles";
	const size_t c_running_in_circles_size_minus_1 = sizeof(c_running_in_circles) - 1;

	const u32 str2_init_capacity = c_running_in_circles_size_minus_1;
	const u32 str2_init_size = 0;

	TestPrint("Creating new array str2... (allocator: %p, capacity: %u)\n", str2_allocator, str2_init_capacity);

	Array<char> str2 = array_new<char>(str2_allocator, str2_init_capacity);

	memcpy(str2.data, c_running_in_circles, c_running_in_circles_size_minus_1);
	str2.size += c_running_in_circles_size_minus_1;

	TestPrint("str2.data: \"");
	TestWrite(str2.data, str2.size);
	TestPrintPrefixed("", "\" (expected: \"%s\")\n", c_running_in_circles);

	TestPrintPrefixed("", "\n");

	// Offset: 0
	// String: R u n n i n g _ i n _ c i r c l e s
	//  Count: 1 2 3 4 5 6 7
	const u32 running_offset = 0;
	const u32 running_count = 7;

	TestPrint("Creating new ArrayView of str2... (array: %p, offset: %u, count: %u)\n", &str2, running_offset, running_count);

	ArrayView<char> str2_view_running = get_array_view(&str2, running_offset, running_count);

	TestPrint("str2_view_running.data: %p (expected: %p)\n", str2_view_running.data, str2.data + running_offset);
	TestAssert(str2_view_running.data == str2.data + running_offset);

	TestPrint("str2_view_running.size: %u (expected: %u)\n", str2_view_running.size, running_count);
	TestAssert(str2_view_running.size == running_count);

	TestPrint("str2_view_running.data: \"");
	TestWrite(str2_view_running.data, str2_view_running.size);
	TestPrintPrefixed("", "\" (expected: \"");
	TestWrite(str2.data + running_offset, running_count);
	TestPrintPrefixed("", "\")\n\n");

	// Offset: 0 1 2 3 4 5 6 7 8
	// String: R u n n i n g _ i n _ c i r c l e s
	//  Count:                 1 2
	const u32 in_offset = 8;
	const u32 in_count = 2;

	TestPrint("Creating new ArrayView of str2... (array: %p, offset: %u, count: %u)\n", &str2, in_offset, in_count);

	ArrayView<char> str2_view_in = get_array_view(&str2, in_offset, in_count);

	TestPrint("str2_view_in.data: %p (expected: %p)\n", str2_view_in.data, str2.data + in_offset);
	TestAssert(str2_view_in.data == str2.data + in_offset);

	TestPrint("str2_view_in.size: %u (expected: %u)\n", str2_view_in.size, in_count);
	TestAssert(str2_view_in.size == in_count);

	TestPrint("str2_view_in.data: \"");
	TestWrite(str2_view_in.data, str2_view_in.size);
	TestPrintPrefixed("", "\" (expected: \"");
	TestWrite(str2.data + in_offset, in_count);
	TestPrintPrefixed("", "\")\n\n");

	//         0 0 0 0 0 0 0 0 0 0 1 1
	// Offset: 0 1 2 3 4 5 6 7 8 9 0 1
	// String: R u n n i n g _ i n _ c i r c l e s
	//  Count:                       1 2 3 4 5 6 7
	const u32 circles_offset = 11;
	const u32 circles_count = 7;

	TestPrint("Creating new ArrayView of str2... (array: %p, offset: %u, count: %u)\n", &str2, circles_offset, circles_count);

	ArrayView<char> str2_view_circles = get_array_view(&str2, circles_offset, circles_count);

	TestPrint("str2_view_circles.data: %p (expected: %p)\n", str2_view_circles.data, str2.data + circles_offset);
	TestAssert(str2_view_circles.data == str2.data + circles_offset);

	TestPrint("str2_view_circles.size: %u (expected: %u)\n", str2_view_circles.size, circles_count);
	TestAssert(str2_view_circles.size == circles_count);

	TestPrint("str2_view_circles.data: \"");
	TestWrite(str2_view_circles.data, str2_view_circles.size);
	TestPrintPrefixed("", "\" (expected: \"");
	TestWrite(str2.data + circles_offset, circles_count);
	TestPrintPrefixed("", "\")\n");


	TestPrintPrefixed("", "\n  --- Array Resize: array_resize() ---\n\n");


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

	TestPrintPrefixed("", "\n  --- Array Popping: array_pop() ---\n\n");

	char stored_last_char = str1.data[str1.size];
	char popped = 0;
	TestAssert(array_pop(&str1, &popped));
	TestAssertMessage(popped == stored_last_char, "Popped array item is different from the expected one");

	stored_last_char = str1.data[str1.size];
	TestAssert(array_pop(&str1, &popped));
	TestAssertMessage(popped == stored_last_char, "Popped array item is different from the expected one");

	TestPrintPrefixed("", "\n  --- Array Searching: array_contains(), array_find() ---\n\n");

	TestAssertMessage(!array_contains(&str1, '\n'), "Array contains item it's not expected to have");

	TestAssert(array_contains(&str1, ' '));
	TestAssert(array_contains(&str1, 'H'));
	TestAssert(array_contains(&str1, 'w'));
	TestAssert(!array_contains(&str1, '!'));
	TestAssert(!array_contains(&str1, 'h'));
	TestAssert(!array_contains(&str1, 'W'));

	char *found = NULL;
	found = array_find(&str1, ' ');
	TestAssert(found);

	found = NULL;
	found = array_find(&str1, 'H');
	TestAssert(found);

	found = NULL;
	found = array_find(&str1, 'w');
	TestAssert(found);

	TestAssert(!array_find(&str1, '!'));
	TestAssert(!array_find(&str1, 'h'));
	TestAssert(!array_find(&str1, 'W'));

	array_clear(&str1);
	TestAssertMessage(str1.size == 0, "Failed to reset array size on clear");

	TestAssert(!array_contains(&str1, ' '));
	TestAssert(!array_contains(&str1, '!'));
	TestAssert(!array_contains(&str1, 'H'));
	TestAssert(!array_contains(&str1, 'h'));
	TestAssert(!array_contains(&str1, 'w'));
	TestAssert(!array_contains(&str1, 'W'));

	TestAssert(!array_find(&str1, ' '));
	TestAssert(!array_find(&str1, '!'));
	TestAssert(!array_find(&str1, 'H'));
	TestAssert(!array_find(&str1, 'h'));
	TestAssert(!array_find(&str1, 'w'));
	TestAssert(!array_find(&str1, 'W'));

	array_clear(&str1);
	TestAssert(str1.size == 0);

	TestPrintPrefixed("", "\n  --- Array Repeated Appending: array_add_repeat() ---\n\n");

	TestAssertMessage(array_add_repeat(&str1, 'q', str1.capacity) == str1.capacity, "Failed to repeatedly add item to array");

	TestAssertMessage(array_add_repeat(&str1, 'w', str1.capacity) == 0, "Added to array when it's full");
	TestAssertMessage(!array_add(&str1, 'w'), "Added to array when it's full");

	array_clear(&str1);
	TestAssert(str1.size == 0);

	TestPrintPrefixed("", "\n  --- Array Appending from other Array: array_add_from_array() ---\n\n");

	const char c_str_hello_world[] = "Hello world!";
	const char c_str_hello[] = "Hello ";

	memcpy(str1.data, c_str_hello, sizeof(c_str_hello) - 1);
	str1.size += sizeof(c_str_hello) - 1;

	Array<char> str3 = array_new<char>(sys_allocator, sizeof(c_str_hello_world) - 1);

	memcpy(str3.data, c_str_hello_world, sizeof(c_str_hello_world) - 1);
	str3.size += sizeof(c_str_hello_world) - 1;

	TestPrint("str1.data: \"");
	TestWrite(str1.data, str1.size);
	TestPrintPrefixed("", "\" (expected: \"%s\")\n", c_str_hello);

	TestPrint("str2.data: \"");
	TestWrite(str2.data, str2.size);
	TestPrintPrefixed("", "\" (expected: \"%s\")\n", c_str_hello_world);

	const u32 source_offset = 6;
	const u32 count = 6;
	// Copy "world" part out of "Hello world!" from str2 to str1.
	TestAssert(array_add_from_array(&str1, &str2, source_offset, count) == count);
	TestPrint("str1.data: \"%s\" (expected: \"%s\")\n", str1.data, "Hello world");

	TestPrintPrefixed("", "\n  --- Array Deallocation: array_free() ---\n\n");

	TestAssertMessage(array_free(&str1), "Failed to free array");

}