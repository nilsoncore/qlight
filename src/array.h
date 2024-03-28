#ifndef QLIGHT_ARRAY_H
#define QLIGHT_ARRAY_H

#include "types.h"
#include "allocator.h"

#define ARRAY_RESIZE_MIN_CAPACITY 64

#define StringViewFormat "%.*s"
#define StringViewArgument(array_view) array_view.size, array_view.data

template <typename T>
struct Array {
	Allocator *allocator;
	u32 size;
	u32 capacity;
	T *data;
};

template <typename T>
struct ArrayView {
	u32 size;
	T *data;
};

template <typename T>
Array<T> array_new(Allocator *allocator, u32 initial_capacity) {
	Array<T> array;
	array.allocator = allocator;
	array.size = 0;
	array.capacity = initial_capacity;
	array.data = (initial_capacity > 0) ? TemplateAllocate(allocator, array.capacity, T) : NULL;
	return array;
}

// count = 0 -- means count to the end of the array's size.
template <typename T>
ArrayView<T> get_array_view(Array<T> *array, u32 offset = 0, u32 count = 0) {
	AssertMessage(array, "ArrayView array pointer is NULL");
	// AssertMessage(offset < array->size, "ArrayView array offset is out of bounds");
	T *view_data = array->data + offset;
	const u32 view_size = (count == 0) ? array->size - offset : count;
	AssertMessage(view_size <= array->size, "ArrayView size is out of bounds");
	ArrayView<T> view = { .size = view_size, .data = view_data };
	return view;
}

#if defined(QLIGHT_MATH_H)
u32 QL_max2(u32 a, u32 b); // from math.h
#else
static u32 QL_max2(u32 a, u32 b) { return (a > b) ? a : b; }
#endif

template <typename T>
u32 array_resize(Array<T> *array, u32 new_capacity) {
	if (new_capacity <= array->size) {
		// Shrink down the size, but keep capacity the same.
		array->size = new_capacity;
		return array->capacity;
	}

	if (new_capacity <= array->capacity) {
		// Shrink down the size, but keep capacity the same.
		array->size = new_capacity;
		return array->capacity;
	}

	// Allocate at least N items.
	// NOTE(nilsoncore): Check if this optimisation is worth it.
	new_capacity = QL_max2(static_cast<u32>(ARRAY_RESIZE_MIN_CAPACITY), new_capacity);
	array->data = TemplateReallocate(array->allocator, array->data, array->capacity, new_capacity, T);
	array->capacity = new_capacity;
	return array->capacity;
}

// Returns newly added item's index.
template <typename T>
u32 array_add(Array<T> *array, T item) {
    if (array->size + 1 > array->capacity) {
    	array_resize(array, array->size * 2);
    }

    array->data[array->size] = item;
    const u32 current_idx = array->size;
    array->size++;
    return array->size - 1;
}

template <typename T>
u32 array_add_repeat(Array<T> *array, T item, u32 count) {
	const u32 space_left = array->capacity - array->size;
	// const u32 items_to_add = (space_left <= count) ? space_left : count - space_left;
	const u32 items_to_add = (space_left <= count) ? space_left : count;
	if (items_to_add < 1)
		return items_to_add;

	for (u32 item_idx = 0; item_idx < items_to_add; item_idx += 1) {
		array->data[item_idx] = item;
	}

	array->size += items_to_add;
	return items_to_add;
}

template <typename T>
u32 array_add_from_array(Array<T> *destination, Array<T> *source, u32 source_offset, u32 count) {
	const bool within_source_array = (source->size >= source_offset + count);
	const u32 source_items_to_add = (within_source_array) ? count : source->size - source_offset - count;
	if (source_items_to_add < 1)
		return source_items_to_add;

	const u32 space_left = destination->capacity - destination->size;
	// const u32 items_to_add = (space_left >= source_items_to_add) ? source_items_to_add : source_items_to_add - space_left;
	const u32 items_to_add = (space_left <= source_items_to_add) ? space_left : source_items_to_add;
	if (items_to_add < 1)
		return items_to_add;

	u32 destination_item_idx = destination->size;
	u32 source_item_idx = source_offset;
	for (u32 item_idx = 0; item_idx < items_to_add; item_idx += 1) {
		destination->data[destination_item_idx] = source->data[source_item_idx];
		destination_item_idx += 1;
		source_item_idx += 1;
	}

	destination->size += items_to_add;
	return items_to_add;
}

template <typename T>
bool array_pop(Array<T> *array, T *out_item) {
    if (array->size < 1)
    	return false;

    if (out_item)
    	*out_item = array->data[array->size - 1];

    array->size--;
    return true;
}

template <typename T>
void array_clear(Array<T> *array, bool zero_memory = false) {
    array->size = 0;
    if (zero_memory)
    	memset(array->data, 0, sizeof(T) * array->capacity);
}

template <typename T>
bool array_free(Array<T> *array) {
    if (!array->data)         return false;
    if (array->capacity < 1)  return false;

    Deallocate(array->allocator, array->data);
    array->size = 0;
    array->capacity = 0;
    return true;
}

template <typename T>
bool array_contains(Array<T> *array, T item) {
    for (u32 index = 0; index < array->size; index++) {
        if (array->data[index] == item)  return true;
    }
    return false;
}

template <typename T>
T* array_find(Array<T> *array, T item) {
    for (u32 index = 0; index < array->size; index++) {
        if (array->data[index] == item)  return &array->data[index];
    }
    return NULL;
}

#endif /* QLIGHT_ARRAY_H */