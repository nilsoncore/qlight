#ifndef QLIGHT_PLATFORM_H
#define QLIGHT_PLATFORM_H

#include "types.h"

void platform_handle_assert(const char *expression, const char *message, const char *file, u32 line);

#endif /* QLIGHT_PLATFORM_H */