#ifndef QLIGHT_ALLOCATOR_H
#define QLIGHT_ALLOCATOR_H

#include "types.h"

#if defined(_WIN32)
#define __ALLOCATOR_CALLER CallerInfo { "(none)", __FUNCSIG__, __FILE__, __LINE__ }
#define __ALLOCATOR_TYPE_CALLER(T) CallerInfo { "'" #T "'", __FUNCSIG__, __FILE__, __LINE__ }
#else
#define __ALLOCATOR_CALLER CallerInfo { "(none)", __PRETTY_FUNCTION__, __FILE__, __LINE__ }
#define __ALLOCATOR_TYPE_CALLER(T) CallerInfo { "'" #T "'", __PRETTY_FUNCTION__, __FILE__, __LINE__ }
#endif

#define ALLOC(allocator, count, T) (T *)allocator->request_allocate(count, sizeof(T), __ALLOCATOR_TYPE_CALLER(T))
#define REALLOC(allocator, memory_pointer, old_count, new_count, T) (T *)allocator->request_reallocate(memory_pointer, old_count, new_count, sizeof(T), __ALLOCATOR_TYPE_CALLER(T))
#define FREE(allocator, memory_pointer) allocator->request_free(memory_pointer, __ALLOCATOR_CALLER)

struct CallerInfo {
	const char *type;
	const char *function;
	const char *file;
	u32 line;
};

struct Allocator {
	u8 *request_allocate(u64 count, u64 size, CallerInfo caller);
	u8 *request_reallocate(void *memory_pointer, u64 old_count, u64 new_count, u64 size, CallerInfo caller);
	void request_free(void *memory_pointer, CallerInfo caller);

	virtual u8 *do_allocate(u64 count, u64 size, CallerInfo caller) = 0;
	virtual u8 *do_reallocate(void *memory_pointer, u64 old_count, u64 new_count, u64 size, CallerInfo caller) = 0;
	virtual void do_free(void *memory_pointer, CallerInfo caller) = 0;
};

extern Allocator *sys_allocator;

struct System_Allocator : Allocator {
	u8 *do_allocate(u64 count, u64 size, CallerInfo caller);
	u8 *do_reallocate(void *memory_pointer, u64 old_count, u64 new_count, u64 size, CallerInfo caller);
	void do_free(void *memory_pointer, CallerInfo caller);
};

struct Linear_Allocator : Allocator {
	u8 *memory_start = NULL;
	u8 *memory_end = NULL;
	u8 *cursor = NULL;
	u8 *cursor_max = NULL;

	u64 allocated = 0;

	u8 *do_allocate(u64 count, u64 size, CallerInfo caller);
	u8 *do_reallocate(void *memory_pointer, u64 old_count, u64 new_count, u64 size, CallerInfo caller);
	void do_free(void *memory_pointer, CallerInfo caller);

	void init(void *memory_pointer, u64 size);
	void deinit();
	void clear(bool zero_memory);

	u64 occupied();
};

#endif /* QLIGHT_ALLOCATOR_H */