#ifndef QLIGHT_ARRAY_H
#define QLIGHT_ARRAY_H

#include "types.h"
#include "allocator.h"

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

template <typename T>
u32 array_resize(Array<T> *array, u32 new_capacity) {
	array->data = TemplateReallocate(array->allocator, array->data, array->capacity, new_capacity, T);
	array->capacity = new_capacity;
	return array->capacity;
}

template <typename T>
bool array_add(Array<T> *array, T item) {
    if (array->size + 1 > array->capacity)  return false;

    array->data[array->size] = item;
    array->size++;
    return true;
}

template <typename T>
T array_pop(Array<T> *array) {
    T item = { };
    if (array->size < 1)  return item;

    item = array->data[array->size];
    array->size--;
    return item;
}

template <typename T>
void array_clear(Array<T> *array) {
    array->size = 0;
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
    for (int index = 0; index < array->size; index++) {
        if (array->data[index] == item)  return true;
    }
    return false;
}

template <typename T>
T* array_find(Array<T> *array, T item) {
    for (int index = 0; index < array->size; index++) {
        if (array->data[index] == item)  return *array->data[index];
    }
    return NULL;
}

#endif /* QLIGHT_ARRAY_H */