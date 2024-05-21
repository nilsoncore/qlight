#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

#include "tests_common.h"

int main(int arguments_count, char *arguments[]) {

	// Windows-specific: Set console's code page to UTF-8
	// so it can render non-ASCII glyphs properly.
	SetConsoleOutputCP(CP_UTF8);

	// Windows-specific(?): Enable UTF-8 C Runtime support. (since Windows 10)
	// Without this call:
	//   1. The MB_CUR_MAX (Current locale MultiByte character Maximum byte size)
	// would return 1 on machine with en-US locale.
	//   2. The console cannot correctly interpret/render glyphs from `fputs()`.
	setlocale(LC_ALL, ".utf8");

	test_allocator();
	test_array();
	test_string();
	return 0;
}