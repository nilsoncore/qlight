// #define QLIGHT_DEBUG

#include "tests_common.h"

#include "../src/array.h"

void test_array() {

	TestPrintPrefixed("", "\n  --- Array Initialization: array_new() ---\n\n");


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
	//
	// TestPrint("str1.data: \"");
	// TestWrite(str1.data, str1.size);
	// TestPrintPrefixed("", "\" (expected: \"\")\n");
	TestPrint("str1.data: \"" StringViewFormat "\", (expected: \"\")\n", StringViewArgument(get_array_view(&str1)));


	TestPrintPrefixed("", "\n  --- Array Appending: array_add() ---\n\n");


	for (u32 char_idx = 0; char_idx < c_hello_size_minus_1; char_idx += 1) {
		const char current_char = c_hello[char_idx];
		TestPrint("Adding char '%c' to str1... - ", current_char);
		u32 added_char_idx = array_add(&str1, current_char);
		bool added = (str1.data[added_char_idx] == current_char);
		TestPrintPrefixed("", "%s.\n", (added) ? "Added" : "Not added");
		TestAssert(added);
	}

	TestPrint("str1.data: \"" StringViewFormat "\" (expected: \"%s\")\n", StringViewArgument(get_array_view(&str1)), c_hello);


	TestPrintPrefixed("", "\n  --- Array View: get_array_view() ---\n\n");


	TestPrint("Creating new ArrayView of str1... (array: %p)\n", &str1);

	ArrayView<char> str1_view_full = get_array_view(&str1); // Full ArrayView

	TestPrint("str1_view_full.data: %p (expected: %p)\n", str1_view_full.data, str1.data);
	TestAssert(str1_view_full.data == str1.data);

	TestPrint("str1_view_full.size: %u (expected: %u)\n", str1_view_full.size, str1.size);
	TestAssert(str1_view_full.size == str1.size);

	TestPrint(
		"str1_view_full.data: \"" StringViewFormat "\" (expected: \"" StringViewFormat "\")\n\n",
		StringViewArgument(str1_view_full), StringViewArgument(get_array_view(&str1))
	);

	Allocator *str2_allocator = sys_allocator;

	const char c_running_in_circles[] = "Running in circles";
	const size_t c_running_in_circles_size_minus_1 = sizeof(c_running_in_circles) - 1;

	const u32 str2_init_capacity = c_running_in_circles_size_minus_1;
	const u32 str2_init_size = 0;

	TestPrint("Creating new array str2... (allocator: %p, capacity: %u)\n", str2_allocator, str2_init_capacity);

	Array<char> str2 = array_new<char>(str2_allocator, str2_init_capacity);

	memcpy(str2.data, c_running_in_circles, c_running_in_circles_size_minus_1);
	str2.size += c_running_in_circles_size_minus_1;

	TestPrint("str2.data: \"" StringViewFormat "\" (expected: \"%s\")\n", StringViewArgument(get_array_view(&str2)), c_running_in_circles);

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
	TestPrintPrefixed("", "\")\n\n");

	//         0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1
	// Offset: 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8
	// String: R u n n i n g _ i n _ c i r c l e s _ _ _
	//  Count:                                     1 2 3
	const u32 offset_overflow_offset = 18;
	const u32 offset_overflow_count = 3;

	u32 offset = offset_overflow_offset;
	u32 count = offset_overflow_count;

	TestPrint("Creating new ArrayView of str2... (array: %p, offset: %u, count: %u)\n", &str2, offset, count);

	ArrayView<char> str2_view_overflow_offset = get_array_view(&str2, offset, count);

	TestPrint("str2_view_overflow_offset.data: %p (expected: %p)\n", str2_view_overflow_offset.data, str2.data + offset);
	TestAssert(str2_view_overflow_offset.data == str2.data + offset);

	TestPrint("str2_view_overflow_offset.size: %u (expected: %u)\n", str2_view_overflow_offset.size, 0);
	TestAssert(str2_view_overflow_offset.size == 0);

	TestPrint("str2_view_overflow_offset.data: \"");
	TestWrite(str2_view_overflow_offset.data, str2_view_overflow_offset.size);
	TestPrintPrefixed("", "\" (expected: \"");
	TestWrite(str2.data + offset, 0);
	TestPrintPrefixed("", "\")\n\n");

	//         0 0 0 0 0 0 0 0 0 0 1 1
	// Offset: 0 1 2 3 4 5 6 7 8 9 0 1
	// String: R u n n i n g _ i n _ c i r c l e s _ _
	//  Count:                       1 2 3 4 5 6 7 8 9
	const u32 count_overflow_offset = 11;
	const u32 count_overflow_count = 9;

	offset = count_overflow_offset;
	count = count_overflow_count;

	TestPrint("Creating new ArrayView of str2... (array: %p, offset: %u, count: %u)\n", &str2, offset, count);

	ArrayView<char> str2_view_overflow_count = get_array_view(&str2, offset, count);

	TestPrint("str2_view_overflow_count.data: %p (expected: %p)\n", str2_view_overflow_count.data, str2.data + offset);
	TestAssert(str2_view_overflow_count.data == str2.data + offset);

	TestPrint("str2_view_overflow_count.size: %u (expected: %u)\n", str2_view_overflow_count.size, 7);
	TestAssert(str2_view_overflow_count.size == 7);

	TestPrint("str2_view_overflow_count.data: \"");
	TestWrite(str2_view_overflow_count.data, str2_view_overflow_count.size);
	TestPrintPrefixed("", "\" (expected: \"");
	TestWrite(str2.data + offset, 7);
	TestPrintPrefixed("", "\")\n\n");

	//         0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1
	// Offset: 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
	// String: R u n n i n g _ i n _ c i r c l e s _ _
	//  Count:                                   1 2 3
	const u32 count_end_overflow_offset = 17;
	const u32 count_end_overflow_count = 3;

	offset = count_end_overflow_offset;
	count = count_end_overflow_count;

	TestPrint("Creating new ArrayView of str2... (array: %p, offset: %u, count: %u)\n", &str2, offset, count);

	ArrayView<char> str2_view_overflow_count_end = get_array_view(&str2, offset, count);

	TestPrint("str2_view_overflow_count_end.data: %p (expected: %p)\n", str2_view_overflow_count_end.data, str2.data + offset);
	TestAssert(str2_view_overflow_count_end.data == str2.data + offset);

	TestPrint("str2_view_overflow_count_end.size: %u (expected: %u)\n", str2_view_overflow_count_end.size, 1);
	TestAssert(str2_view_overflow_count_end.size == 1);

	TestPrint("str2_view_overflow_count_end.data: \"");
	TestWrite(str2_view_overflow_count_end.data, str2_view_overflow_count_end.size);
	TestPrintPrefixed("", "\" (expected: \"");
	TestWrite(str2.data + offset, 1);
	TestPrintPrefixed("", "\")\n");


	TestPrintPrefixed("", "\n  --- Array Resize: array_resize() ---\n\n");


	TestPrint("Before resize:\n");
	TestPrint("str1.size: %u\n", str1.size);
	TestPrint("str1.capacity: %u\n", str1.capacity);
	TestPrint("str1.data: \"" StringViewFormat "\"\n\n", StringViewArgument(get_array_view(&str1)));

	const u32 grow_by = 7;
	const u32 before_grow_size = str1.size;
	const u32 before_grow_capacity = str1.capacity;
	const u32 grow_size = before_grow_capacity + grow_by;
	u32 expected_capacity = QL_max2(ARRAY_RESIZE_MIN_CAPACITY, grow_size);

	TestPrint("Calling array_resize() with new_capacity: %u\n", grow_size);
	array_resize(&str1, grow_size);
	TestPrint("Resized str1 capacity to: %u (expected: %u) (ARRAY_RESIZE_MIN_CAPACITY: %u)\n\n", str1.capacity, expected_capacity, ARRAY_RESIZE_MIN_CAPACITY);
	TestAssertMessage(str1.capacity == expected_capacity, "Array capacity is different from the one expected from resize");

	TestPrint("After resize:\n");
	TestPrint("str1.size: %u\n", str1.size);
	TestPrint("str1.capacity: %u\n", str1.capacity);
	TestPrint("str1.data: \"" StringViewFormat "\"\n\n", StringViewArgument(get_array_view(&str1)));


	const char c_world[] = " world!";
	const size_t c_world_size_minus_1 = sizeof(c_world) - 1;
	for (u32 char_idx = 0; char_idx < c_world_size_minus_1; char_idx += 1) {
		const char current_char = c_world[char_idx];
		TestPrint("Adding char '%c' to str1... - ", current_char);
		u32 added_char_idx = array_add(&str1, current_char);
		bool added = (str1.data[added_char_idx] == current_char);
		TestPrintPrefixed("", "%s.\n", (added) ? "Added" : "Not added");
		TestAssert(added);
	}

	// TODO: make something like `c_string()` that return the string zero-terminated string
	// that is safe to pass to C functions; OR/AND make custom functions that support this type
	// of `string` naturally.

	const u32 hello_offset = 0;
	const u32 hello_count = 5;
	ArrayView<char> str1_view_hello = get_array_view(&str1, 0, 5); // Partial ArrayView
	TestAssertMessage(str1_view_hello.data == str1.data + hello_offset, "Partial ArrayView points to wrong array memory");
	TestAssertMessage(str1_view_hello.size == hello_count, "Partial ArrayView size is different from given one");

	TestPrint("str1.data: \"" StringViewFormat "\"\n", StringViewArgument(get_array_view(&str1)));
	TestPrint("str1_view_hello.data: \"" StringViewFormat "\"\n", StringViewArgument(str1_view_hello));


	TestPrintPrefixed("", "\n  --- Array Popping: array_pop() ---\n\n");


	TestPrint("str1.data: \"" StringViewFormat "\"\n", StringViewArgument(get_array_view(&str1)));

	char stored_last_char = str1.data[str1.size - 1];
	char popped = 0;

	TestPrint("Last item of str1: '%c'\n", stored_last_char);
	TestPrint("Popping last item from str1... - ");
	TestAssert(array_pop(&str1, &popped));
	TestPrintPrefixed("", "Popped '%c'.\n", popped);

	TestAssertMessage(popped == stored_last_char, "Popped array item is different from the expected one");

	stored_last_char = str1.data[str1.size - 1];

	TestPrint("Last item of str1: '%c'\n", stored_last_char);
	TestPrint("Popping last item from str1... - ");
	TestAssert(array_pop(&str1, &popped));
	TestPrintPrefixed("", "Popped '%c'.\n", popped);

	TestAssertMessage(popped == stored_last_char, "Popped array item is different from the expected one");

	TestPrint("str1.data: \"" StringViewFormat "\"\n", StringViewArgument(get_array_view(&str1)));


	TestPrintPrefixed("", "\n  --- Array Searching: array_contains(), array_find() ---\n\n");


	TestPrint("str1.data: \"" StringViewFormat "\"\n\n", StringViewArgument(get_array_view(&str1)));

	bool contains = false;

	contains = array_contains(&str1, '\n');
	TestPrint("str1 contains '%s': %s (expected: %s)\n", "\\n", QL_bool_to_string(contains), QL_bool_to_string(false));
	TestAssert(!contains);

	contains = array_contains(&str1, ' ');
	TestPrint("str1 contains '%c': %s (expected: %s)\n", ' ', QL_bool_to_string(contains), QL_bool_to_string(true));
	TestAssert(contains);

	contains = array_contains(&str1, 'H');
	TestPrint("str1 contains '%c': %s (expected: %s)\n", 'H', QL_bool_to_string(contains), QL_bool_to_string(true));
	TestAssert(contains);

	contains = array_contains(&str1, 'w');
	TestPrint("str1 contains '%c': %s (expected: %s)\n", 'w', QL_bool_to_string(contains), QL_bool_to_string(true));
	TestAssert(contains);

	contains = array_contains(&str1, '!');
	TestPrint("str1 contains '%c': %s (expected: %s)\n", '!', QL_bool_to_string(contains), QL_bool_to_string(false));
	TestAssert(!contains);

	contains = array_contains(&str1, 'h');
	TestPrint("str1 contains '%c': %s (expected: %s)\n", 'h', QL_bool_to_string(contains), QL_bool_to_string(false));
	TestAssert(!contains);

	contains = array_contains(&str1, 'W');
	TestPrint("str1 contains '%c': %s (expected: %s)\n\n", 'W', QL_bool_to_string(contains), QL_bool_to_string(false));
	TestAssert(!contains);

	char *found;

	found = NULL;
	found = array_find(&str1, '\n');
	TestPrint("Found '%s' in str1: %s (expected: %s)\n", "\\n", QL_bool_to_string(found), QL_bool_to_string(false));
	TestAssert(!found);

	found = NULL;
	found = array_find(&str1, ' ');
	TestPrint("Found '%c' in str1: %s (expected: %s)\n", ' ', QL_bool_to_string(found), QL_bool_to_string(true));
	TestAssert(found);

	found = NULL;
	found = array_find(&str1, 'H');
	TestPrint("Found '%c' in str1: %s (expected: %s)\n", 'H', QL_bool_to_string(found), QL_bool_to_string(true));
	TestAssert(found);

	found = NULL;
	found = array_find(&str1, 'w');
	TestPrint("Found '%c' in str1: %s (expected: %s)\n", 'w', QL_bool_to_string(found), QL_bool_to_string(true));
	TestAssert(found);

	found = (char *) 1; // To make sure array_find() returns NULL when item is not found.
	found = array_find(&str1, '!');
	TestPrint("Found '%c' in str1: %s (expected: %s)\n", '!', QL_bool_to_string(found), QL_bool_to_string(false));
	TestAssert(!found);

	found = (char *) 1;
	found = array_find(&str1, 'h');
	TestPrint("Found '%c' in str1: %s (expected: %s)\n", 'h', QL_bool_to_string(found), QL_bool_to_string(false));
	TestAssert(!found);

	found = (char *) 1;
	found = array_find(&str1, 'W');
	TestPrint("Found '%c' in str1: %s (expected: %s)\n\n", 'W', QL_bool_to_string(found), QL_bool_to_string(false));
	TestAssert(!found);


	TestPrint("str1.data: %p\n", str1.data);
	TestPrint("str1.size: %u\n", str1.size);
	TestPrint("str1.capacity: %u\n", str1.capacity);

	TestPrint("Clearing str1 array...\n");
	array_clear(&str1);

	TestPrint("str1.data: %p\n", str1.data);
	TestPrint("str1.size: %u\n", str1.size);
	TestPrint("str1.capacity: %u\n", str1.capacity);

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


	TestPrintPrefixed("", "\n  --- Array Repeated Appending: array_add_repeat() ---\n\n");


	TestAssertMessage(array_add_repeat(&str1, 'q', str1.capacity) == str1.capacity, "Failed to repeatedly add item to array");

	TestAssertMessage(array_add_repeat(&str1, 'w', str1.capacity) == 0, "Added to array when it's full");

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

	TestPrint("str1.data: \"" StringViewFormat "\" (expected: \"%s\")\n", StringViewArgument(get_array_view(&str1)), c_str_hello);

	TestPrint("str3.data: \"" StringViewFormat "\" (expected: \"%s\")\n", StringViewArgument(get_array_view(&str3)), c_str_hello_world);

	{
		const u32 source_offset = 6;
		const u32 count = 5;
		// Copy "world" part out of "Hello world!" from str3 to str1.
		TestAssert(array_add_from_array(&str1, &str3, source_offset, count) == count);
		TestPrint("str1.data: \"" StringViewFormat "\" (expected: \"%s\")\n", StringViewArgument(get_array_view(&str1)), "Hello world");

		TestPrintPrefixed("", "\n  --- Array Deallocation: array_free() ---\n\n");

		TestAssertMessage(array_free(&str1), "Failed to free array");
	}
}