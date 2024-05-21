#include "tests_common.h"

#include "../src/allocator.h"

void test_allocator() {

	char *buffer = Allocate(sys_allocator, 16, char);
	TestAssertMessage(buffer != NULL, "Failed to heap allocate buffer memory.");

	buffer = Reallocate(sys_allocator, buffer, 16, 32, char);
	TestAssertMessage(buffer != NULL, "Failed to heap reallocate buffer memory.");

	Deallocate(sys_allocator, buffer);

	Linear_Allocator linear_allocator;
	u8 *arena_memory = Allocate(sys_allocator, 1024, u8);
	TestAssertMessage(arena_memory != NULL, "Failed to heap allocate buffer memory for linear allocator.");

	linear_allocator.init(arena_memory, 1024);

	Allocator *a = &linear_allocator;

	char *buffer1 = Allocate(a, 32, char);
	TestAssertMessage(buffer1 != NULL, "Failed to linear allocate buffer memory.");

	const char *msg1 = "Hello, world!";
	memcpy(buffer1, msg1, strlen(msg1) + 1);
	TestAssertMessage(strcmp(buffer1, msg1) == 0, "Failed to copy string into linear allocated buffer memory.");

	TestPrint("1 - buffer1 pointer: 0x%p\n", buffer1);
	TestPrint("1 - buffer1  string: '%s'\n", buffer1);

	char *buffer2 = Allocate(a, 32, char);
	TestAssertMessage(buffer2 != NULL, "Failed to linear allocate buffer memory.");

	const char *msg2 = "Goodbye, sadness!";
	memcpy(buffer2, msg2, strlen(msg2) + 1);
	TestAssertMessage(strcmp(buffer2, msg2) == 0, "Failed to copy string into linear allocated buffer memory.");

	TestPrint("2 - buffer2 pointer: 0x%p\n", buffer2);
	TestPrint("2 - buffer2  string: '%s'\n", buffer2);
	TestPrint("2 - diff(buffer1, buffer2): %lld\n", (s64)(buffer1 - buffer2));

	buffer1 = Reallocate(a, buffer1, 32, 64, char);
	TestAssertMessage(buffer1 != NULL, "Failed to linear reallocate buffer memory.");

	const char *msg3 = "What's up, people!";
	memcpy(buffer1, msg3, strlen(msg3) + 1);
	TestAssertMessage(strcmp(buffer1, msg3) == 0, "Failed to copy string into linear reallocated buffer memory.");

	TestPrint("3 - buffer1 pointer: 0x%p\n", buffer1);
	TestPrint("3 - buffer1  string: '%s'\n", buffer1);
	TestPrint("3 - diff(buffer1, buffer2): %lld\n", (s64)(buffer1 - buffer2));

	linear_allocator.reset(false);
	TestAssertMessage(linear_allocator.occupied() == 0, "Failed to reset linear allocator memory.");
}