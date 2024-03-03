#include "tests_common.h"

#include "../src/allocator.h"

void test_allocator() {
	char *buffer = ALLOC(sys_allocator, 16, char);
	assert(buffer != NULL);

	buffer = REALLOC(sys_allocator, buffer, 16, 32, char);
	assert(buffer != NULL);

	FREE(sys_allocator, buffer);

	Linear_Allocator linear_allocator;
	u8 *arena_memory = ALLOC(sys_allocator, 1024, u8);
	assert(arena_memory != NULL);

	linear_allocator.init(arena_memory, 1024);

	Allocator *a = &linear_allocator;

	char *buffer1 = ALLOC(a, 32, char);
	assert(buffer1 != NULL);

	const char *msg1 = "Hello, world!";
	memcpy(buffer1, msg1, strlen(msg1)+1);
	assert(strcmp(buffer1, msg1) == 0);

	printf("1 - buffer1 pointer: 0x%p\n", buffer1);
	printf("1 - buffer1  string: '%s'\n", buffer1);

	char *buffer2 = ALLOC(a, 32, char);
	assert(buffer2 != NULL);

	const char *msg2 = "Goodbye, sadness!";
	memcpy(buffer2, msg2, strlen(msg2)+1);
	assert(strcmp(buffer2, msg2) == 0);

	printf("2 - buffer2 pointer: 0x%p\n", buffer2);
	printf("2 - buffer2  string: '%s'\n", buffer2);
	printf("2 - diff(buffer1, buffer2): %lld\n", (s64)(buffer1 - buffer2));

	buffer1 = REALLOC(a, buffer1, 32, 64, char);
	assert(buffer1 != NULL);

	const char *msg3 = "What's up, people!";
	memcpy(buffer1, msg3, strlen(msg3)+1);
	assert(strcmp(buffer1, msg3) == 0);

	printf("3 - buffer1 pointer: 0x%p\n", buffer1);
	printf("3 - buffer1  string: '%s'\n", buffer1);
	printf("3 - diff(buffer1, buffer2): %lld\n", (s64)(buffer1 - buffer2));

	linear_allocator.clear(false);
	assert(linear_allocator.occupied() == 0);
}