#pragma once

#include <od/core/module.h>

struct odType;
struct odRange;
struct odAllocation;
struct odArray;
struct odString;

struct odType {
	int32_t size;
	void (*default_construct_fn)(void* ptr, int32_t count);
	void (*move_assign_fn)(void* ptr, void* src_ptr, int32_t count);
	void (*destruct_fn)(void* ptr, int32_t count);
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odType_get_valid(const struct odType* type);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const char* odType_get_debug_string(const struct odType* type);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
void* odType_index(const struct odType* type, void* array, int32_t i);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const void* odType_index_const(const struct odType* type, const void* array, int32_t i);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const struct odType* odType_get_char(void);


OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const struct odType* odAllocation_get_type_constructor(void);

OD_API_C OD_CORE_MODULE
void odAllocation_swap(struct odAllocation* allocation1, struct odAllocation* allocation2);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const char* odAllocation_get_debug_string(const struct odAllocation* allocation);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odAllocation_get_valid(const struct odAllocation* allocation);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odAllocation_init(struct odAllocation* allocation, int32_t allocation_size);

OD_API_C OD_CORE_MODULE
void odAllocation_destroy(struct odAllocation* allocation);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
void* odAllocation_get(struct odAllocation* allocation);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const void* odAllocation_get_const(const struct odAllocation* allocation);



OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const struct odType* odArray_get_type_constructor(void);

OD_API_C OD_CORE_MODULE
void odArray_swap(struct odArray* array1, struct odArray* array2);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odArray_get_valid(const struct odArray* array);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const char* odArray_get_debug_string(const struct odArray* array);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odArray_init(struct odArray* array, const struct odType* type);

OD_API_C OD_CORE_MODULE
void odArray_destroy(struct odArray* array);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const struct odType* odArray_get_type(const struct odArray* array);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
int32_t odArray_get_capacity(const struct odArray* array);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odArray_set_capacity(struct odArray* array, int32_t new_capacity);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odArray_ensure_capacity(struct odArray* array, int32_t min_capacity);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
int32_t odArray_get_count(const struct odArray* array);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odArray_set_count(struct odArray* array, int32_t new_count);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odArray_expand(struct odArray* array, void** out_expand_dest, int32_t expand_count);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odArray_push(struct odArray* array, void* moved_src, int32_t moved_count);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odArray_pop(struct odArray* array, int32_t count);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odArray_swap_pop(struct odArray* array, int32_t i);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
void* odArray_get(struct odArray* array, int32_t i);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const void* odArray_get_const(const struct odArray* array, int32_t i);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
void* odArray_begin(struct odArray* array);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const void* odArray_begin_const(const struct odArray* array);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
void* odArray_end(struct odArray* array);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const void* odArray_end_const(const struct odArray* array);


OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const struct odType* odString_get_type_constructor(void);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odString_copy(struct odString* string, const struct odString* src_string);

OD_API_C OD_CORE_MODULE
void odString_swap(struct odString* string1, struct odString* string2);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odString_get_valid(const struct odString* string);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
int32_t odString_compare(const struct odString* string1, const struct odString* string2);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const char* odString_get_debug_string(const struct odString* string);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odString_init(struct odString* string);

OD_API_C OD_CORE_MODULE
void odString_destroy(struct odString* string);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
int32_t odString_get_capacity(const struct odString* string);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odString_set_capacity(struct odString* string, int32_t new_capacity);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odString_ensure_capacity(struct odString* string, int32_t min_capacity);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
int32_t odString_get_count(const struct odString* string);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odString_set_count(struct odString* string, int32_t new_count);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odString_expand(struct odString* string, char** out_expand_dest, int32_t expand_count);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odString_push(struct odString* string, const char* str, int32_t str_count);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odString_push_formatted_variadic(struct odString* string, const char* format_c_str, va_list args);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odString_push_formatted(struct odString* string, const char* format_c_str, ...);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odString_ensure_null_terminated(struct odString* string);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odString_get_null_terminated(const struct odString* string);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
char* odString_get(struct odString* string, int32_t i);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const char* odString_get_const(const struct odString* string, int32_t i);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
char* odString_begin(struct odString* string);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const char* odString_begin_const(const struct odString* string);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
char* odString_end(struct odString* string);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const char* odString_end_const(const struct odString* string);
