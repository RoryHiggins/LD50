#pragma once

#include <od/core/api.h>

struct odType;
struct odArray;

OD_API_C const struct odType* odArray_get_type_constructor(void);
OD_API_C void odArray_swap(struct odArray* array1, struct odArray* array2);
OD_API_C const char* odArray_get_debug_string(const struct odArray* array);
OD_API_C const struct odType* odArray_get_type(const struct odArray* array);
OD_API_C void odArray_set_type(struct odArray* array, const struct odType* type);
OD_API_C uint32_t odArray_get_capacity(const struct odArray* array);
OD_API_C bool odArray_set_capacity(struct odArray* array, uint32_t new_capacity);
OD_API_C bool odArray_ensure_capacity(struct odArray* array, uint32_t min_capacity);
OD_API_C void odArray_release(struct odArray* array);
OD_API_C uint32_t odArray_get_count(const struct odArray* array);
OD_API_C bool odArray_set_count(struct odArray* array, uint32_t new_count);
OD_API_C bool odArray_expand(struct odArray* array, void** out_expand_dest, uint32_t expand_count);
OD_API_C bool odArray_push(struct odArray* array, void* moved_src, uint32_t moved_count);
OD_API_C bool odArray_pop(struct odArray* array, uint32_t count);
OD_API_C bool odArray_swap_pop(struct odArray* array, uint32_t i);
OD_API_C void* odArray_get(struct odArray* array, uint32_t i);
OD_API_C const void* odArray_get_const(const struct odArray* array, uint32_t i);
