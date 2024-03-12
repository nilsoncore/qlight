#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS

#include "platform.h"
#include <stdio.h> // snprintf()
#include <stdlib.h> // exit()

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <Windows.h>

void platform_assert_fail(const char *expression, const char *message, const char *file, long line) {
	const char title[] = "Assertion failed";
	const size_t title_length_minus_1 = sizeof(title) - 1;

	const size_t buffer_size = 2048;
	char text[buffer_size];
	size_t cursor = 0;
	cursor += snprintf(
		text, buffer_size,
		"%s!\n"
		"Expression: %s\n"
		"Message: %s\n"
		"File: %s:%ld\n",
		title, expression, message, file, line
	);

	fputs(text, stderr);

	if (IsDebuggerPresent()) {
		DebugBreak();
		return;
	}

	const char attach[] =
		"\n"
		"Attach a debugger to the process and press \"Retry\" to start debugging, or press \"Cancel\" to close the program.";
	const size_t attach_length = sizeof(attach);

	// Warning: cursor position is not updated after this!
	strncat(
		/* destination */ text + cursor,
		/*      source */ attach,
		/*       count */ attach_length
	);

	const char *text_skipped_title = text + title_length_minus_1 + 2;
	int pressed_button_id = MessageBoxA(
		/* HWND        hWnd */ NULL,
		/* LPCSTR    lpText */ text_skipped_title,
		/* LPCSTR lpCaption */ title,
		/* UINT       uType */ MB_ICONERROR | MB_RETRYCANCEL | MB_SETFOREGROUND | MB_TASKMODAL
	);

	switch (pressed_button_id) {
		case IDRETRY: {
			DebugBreak();
			break;
		}
		case IDCANCEL:
		default: {
			exit(EXIT_FAILURE);
			break;
		}
	}
}

#endif /* _WIN32 */
