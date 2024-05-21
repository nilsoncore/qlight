// WARNING:
// This file contains UTF-8 characters, therefore it has to be
// encoded in UTF-8. But apparently Microsoft compiler doesn't
// understand it as such without byte-order marking (BOM),
// that's why this file has to be encoded in UTF-8 with BOM.

// TODO(nilsonragee): Factor this out.
#include <stdlib.h> // mblen(), MB_CUR_MAX, MB_LEN_MAX

#include "tests_common.h"

#include "../src/string.h"

String test_string_new(const char *name, Allocator *allocator, u32 initial_capacity_in_bytes) {
	TestPrint("Creating new string `%s`... (allocator: %p, capacity: %u)\n", name, allocator, initial_capacity_in_bytes);

	String string = string_new(allocator, initial_capacity_in_bytes);

	TestPrint("%s.allocator: %p (expected: %p)\n", name, string.allocator, allocator);
	TestAssert(string.allocator == allocator);

	TestPrint("%s.capacity: %u (expected: %u)\n", name, string.capacity, initial_capacity_in_bytes);
	TestAssert(string.capacity == initial_capacity_in_bytes);

	TestPrint("%s.size: %u (expected: %u)\n", name, string.size, 0);
	TestAssert(string.size == 0);

	TestPrint("%s.size_in_bytes: %u (expected: %u)\n", name, string.size_in_bytes, 0);
	TestAssert(string.size_in_bytes == 0);

	TestPrint("%s.data: %p (expected: not NULL)\n\n", name, string.data);
	TestAssert(string.data);

	TestPrint("str1.data: \"" StringFormat "\", (expected: \"\")\n", StringArgumentValue(string));

	return string;
}

void test_string() {

	TestPrintPrefixed("", "\n  --- String Initialization: string_new() ---\n\n");

	TestPrint("MB_CUR_MAX: %d bytes\n", MB_CUR_MAX);
	TestPrint("MB_LEN_MAX: %d bytes\n", MB_LEN_MAX);
	TestPrint("sizeof(char8_t): %llu bytes\n", sizeof(char8_t));
	TestPrint("sizeof(char16_t): %llu bytes\n", sizeof(char16_t));
	TestPrint("sizeof(char32_t): %llu bytes\n\n", sizeof(char32_t));

	Allocator *str1_allocator = sys_allocator;
	const u32 str1_init_capacity = 16;

	String str1 = test_string_new("str1", str1_allocator, str1_init_capacity);


	TestPrintPrefixed("", "\n  --- String Appending: string_add() ---\n\n");


	const char c_hello[] = "Hello";
	const size_t c_hello_size_minus_1 = sizeof(c_hello) - 1;

	// Add ASCII "Hello" chars one by one.
	{
		for (u32 char_idx = 0; char_idx < c_hello_size_minus_1; char_idx += 1) {
			const char current_char = c_hello[char_idx];
			TestPrint("Adding ASCII char '%c' (1 byte) to `str1`... - ", current_char);
			u32 added_char_idx = string_add(&str1, current_char);
			bool added = (str1.data[added_char_idx] == current_char);
			TestPrintPrefixed("", "%s.\n", (added) ? "Added" : "Not added");
			TestAssert(added);
		}
	}

	TestPrintPrefixed("", "\n");
	TestPrint("str1.data: \"" StringViewFormat "\" (expected: \"%s\")\n", StringViewArgument(get_array_view(&str1)), c_hello);

	const char8_t c_utf8_ru_hello[] = u8" Привет";
	const u32 c_utf8_ru_hello_size_minus_1 = sizeof(c_utf8_ru_hello) - 1;

	// Add UTF-8 " Привет" chars in one go.
	{
		const char8_t expected_str[] = u8"Hello Привет";
		const u32 expected_str_size_minus_1 = sizeof(expected_str) - 1;
		u32 cursor = string_add(&str1, (char8_t *)&c_utf8_ru_hello, c_utf8_ru_hello_size_minus_1);

		TestPrint("str1.size: %u\n", str1.size);
		TestPrint("str1.size_in_bytes: %u\n", str1.size_in_bytes);
		TestPrint("str1.data: \"");
		TestWrite(str1.data, str1.size_in_bytes);
		TestPrintPrefixed("", "\" (expected: \"");
		TestWrite((const char *)expected_str, expected_str_size_minus_1);
		TestPrintPrefixed("", "\")\n\n");
	}

	const char8_t c_utf8_jp_hello[] = u8" こんにちは 𝄞";
	const u32 c_utf8_jp_hello_size_minus_1 = sizeof(c_utf8_jp_hello) - 1;

	// Add UTF-8 " こんにちは 𝄞" chars one by one.
	{
		u32 char_idx = 0;
		while (char_idx < c_utf8_jp_hello_size_minus_1) {
			const char8_t *current_utf8_char = &c_utf8_jp_hello[char_idx];
			TestPrint("Adding UTF-8 char '");
			const char *c_utf8_jp_hello_cursor = (const char *)&c_utf8_jp_hello[char_idx];
			int current_utf8_char_size = mblen(c_utf8_jp_hello_cursor, MB_CUR_MAX);
			TestWrite(c_utf8_jp_hello_cursor, current_utf8_char_size);
			TestPrintPrefixed("", "' (%d byte%c) to `str1`... - ", current_utf8_char_size, (current_utf8_char_size == 1) ? '\0' : 's');
			u32 added_char_idx = string_add_multibyte_char(&str1, (char8_t *)current_utf8_char, current_utf8_char_size);
			bool added = (str1.data[added_char_idx] == *current_utf8_char);
			TestPrintPrefixed("", "%s.\n", (added) ? "Added" : "Not added");
			TestAssert(added);

			char_idx += current_utf8_char_size;
		}

		const char8_t expected_str[] = u8"Hello Привет こんにちは 𝄞";
		const u32 expected_str_size_minus_1 = sizeof(expected_str) - 1;

		TestPrintPrefixed("", "\n");
		TestPrint("str1.size: %u\n", str1.size);
		TestPrint("str1.size_in_bytes: %u\n", str1.size_in_bytes);
		TestPrint("str1.data: \"");
		TestWrite(str1.data, str1.size_in_bytes);
		TestPrintPrefixed("", "\" (expected: \"");
		TestWrite((const char *)expected_str, expected_str_size_minus_1);
		TestPrintPrefixed("", "\")\n");
	}


	TestPrintPrefixed("", "\n  --- String Popping: string_pop() ---\n\n");


	{
		StringChar last_char = string_pop(&str1, /* do_pop = */ true);
		TestPrint("str1.data: \"" StringFormat "\"\n", StringArgumentValue(str1));
		TestPrint("last_char.size_in_bytes: %u\n", last_char.size_in_bytes);
		TestPrint("Popped: '");
		TestWrite(last_char.bytes, last_char.size_in_bytes);
		// TestPrintPrefixed("", "%.*s", last_char.size_in_bytes, last_char.bytes);
		TestPrintPrefixed("", "' (size: %u bytes)\n", last_char.size_in_bytes);
	}


	TestPrintPrefixed("", "\n  --- String Peeking: string_peek() ---\n\n");


	{
		String str1_peek = string_new(sys_allocator, str1.capacity);
		u32 string_characters_read = 0;
		u32 string_bytes_read = 0;
		mbstate_t multibyte_shift_state = { };

		TestPrint("Peeking whole `str1` string passing shift state:\n");
		TestPrint("\"");

		StringChar character = string_peek(&str1, 0, &multibyte_shift_state);
		while (character.size_in_bytes != 0) {
			if (character.size_in_bytes > 0) {
				// Valid UTF-8 character.
				TestWrite(character.bytes, character.size_in_bytes);
				string_add_multibyte_char(&str1_peek, character.bytes, character.size_in_bytes);
			} else {
				// Encoding is not valid, not a UTF-8 character.
				TestWrite("?", 1);
				string_add(&str1_peek, '?');
				string_bytes_read = -character.size_in_bytes - 1; // Just so we advance by one byte.
			}

			string_characters_read += 1;
			string_bytes_read += character.size_in_bytes;
			character = string_peek(&str1, string_bytes_read, &multibyte_shift_state);
		}

		const char8_t expected_str[] = u8"Hello Привет こんにちは 𝄞";
		const u32 expected_str_size_minus_1 = sizeof(expected_str) - 1;

		TestPrintPrefixed("", "\". Read: %u characters, %u bytes\n", string_characters_read, string_bytes_read);
		TestPrint(
			"Saved peeked characters string: \"" StringFormat "\" (expected: \"%s\")\n",
			StringArgumentValue(str1_peek),
			expected_str
		);
	}

	{
		String str1_peek = string_new(sys_allocator, str1.capacity);
		u32 string_characters_read = 0;
		u32 string_bytes_read = 0;

		TestPrint("Peeking whole `str1` string not passing shift state:\n");
		TestPrint("\"");

		StringChar character = string_peek(&str1, 0);
		while (character.size_in_bytes != 0) {
			if (character.size_in_bytes > 0) {
				// Valid UTF-8 character.
				TestWrite(character.bytes, character.size_in_bytes);
				string_add_multibyte_char(&str1_peek, character.bytes, character.size_in_bytes);
			} else {
				// Encoding is not valid, not a UTF-8 character.
				TestWrite("?", 1);
				string_add(&str1_peek, '?');
				string_bytes_read = -character.size_in_bytes - 1; // Just so we advance by one byte.
			}

			string_characters_read += 1;
			string_bytes_read += character.size_in_bytes;
			character = string_peek(&str1, string_bytes_read);
		}

		const char8_t expected_str[] = u8"Hello Привет こんにちは 𝄞";
		const u32 expected_str_size_minus_1 = sizeof(expected_str) - 1;

		TestPrintPrefixed("", "\". Read: %u characters, %u bytes\n", string_characters_read, string_bytes_read);
		TestPrint(
			"Saved peeked characters string: \"" StringFormat "\" (expected: \"%s\")\n",
			StringArgumentValue(str1_peek),
			expected_str
		);
	}
}
