#ifndef QLIGHT_STRING_H
#define QLIGHT_STRING_H

#include "array.h"

#define QLIGHT_UNICODE_STRING_MAX_BYTES 4
#define QLIGHT_STRING_PEEK_END U32_MAX

#undef StringFormat
#define StringFormat "%.*s"

#define StringArgumentValue(string) string.size_in_bytes, string.data
#define StringArgumentPointer(string) string->size_in_bytes, string->data

struct String_UTF8 : Array<char8_t> {
	u32 size_in_bytes;
};

struct StringView_UTF8 : ArrayView<char8_t> {
	u32 size_in_bytes;
};

struct StringChar {
	s32 size_in_bytes;
	char8_t bytes[QLIGHT_UNICODE_STRING_MAX_BYTES];
};

typedef     String_UTF8 String;
typedef StringView_UTF8 StringView;

String string_new(Allocator *allocator, u32 initial_capacity_in_bytes);
String string_new(Allocator *allocator, const char8_t *c_utf8_string, u32 c_utf8_string_size = 0);
StringView get_string_view(String *string, u32 offset = 0, u32 length = 0);

// Append ASCII (1 byte long) character `ascii_char` to `string`.
// Returns the index of character's first byte inside a string.
u32 string_add(String *string, char8_t ascii_char);

// Append UTF-8 (1-4 bytes long) character to a string.
// Returns the index of character's first byte inside a string.
u32 string_add_multibyte_char(String *string, char8_t *utf8_character_array, u32 character_size_in_bytes);

// Append UTF-8 (1-4 bytes long) character sequence (one or more characters) to a string.
// Returns the index of last added character's first byte inside a string.
u32 string_add(String *string, char8_t *utf8_char_array, u32 array_size_in_bytes, u32 *utf8_characters_added = NULL);

// Pop last character from `string`.
// Returns `StringChar` stucture which contains character data.
StringChar string_pop(String *string, bool do_pop = true);

// Peek a character from `string`
StringChar string_peek(String *string, u32 offset_in_bytes);
StringChar string_peek(String *string, u32 offset_in_bytes, mbstate_t *multibyte_shift_state);

/* TODO */


#endif /* QLIGHT_STRING_H */
