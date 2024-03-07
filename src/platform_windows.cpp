#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <Windows.h>

void platform_handle_assert(const char *expression, const char *message, const char *file, u32 line) {
	char text[1024];
	text[0] = '\0';

	u64 cursor = 0;
	cursor += snprintf(
		"Expression: %s\n"
		"Message: %s\n"
		"File: %s:%u\n"
		"\n"
		"Press \"Retry\" to debug code or \"Cancel\" to close the program.",
		expression, message, file, line
	);

	int button_id = MessageBoxA(
		/* HWND        hWnd */ NULL,
		/* LPCSTR    lpText */ text,
		/* LPCSTR lpCaption */ "Assertion failed",
		/* UINT       uType */ MB_ICONERROR | MB_RETRYCANCEL | MB_SETFOREGROUND | MB_TASKMODAL
	);

	switch (button_id) {
		case IDRETRY: {
			break;
		}
		case IDCANCEL:
		default: {
			exit(1);
			break;
		}
	}
}

#endif /* _WIN32 */