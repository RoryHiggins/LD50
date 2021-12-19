#pragma once

#include <od/core/module.h>

struct odType;

struct odTrivialArray;
struct odArray;

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odType*
odTrivialArray_get_type_constructor(void);
OD_API_C OD_CORE_MODULE void
odTrivialArray_swap(struct odTrivialArray* array1, struct odTrivialArray* array2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odTrivialArray_check_valid(const struct odTrivialArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odTrivialArray_get_debug_string(const struct odTrivialArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odTrivialArray_init(struct odTrivialArray* array);
OD_API_C OD_CORE_MODULE void
odTrivialArray_destroy(struct odTrivialArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD int32_t
odTrivialArray_get_capacity(const struct odTrivialArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odTrivialArray_set_capacity(struct odTrivialArray* array, int32_t new_capacity, int32_t stride);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odTrivialArray_ensure_capacity(struct odTrivialArray* array, int32_t min_capacity, int32_t stride);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD int32_t
odTrivialArray_get_count(const struct odTrivialArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odTrivialArray_set_count(struct odTrivialArray* array, int32_t new_count, int32_t stride);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odTrivialArray_extend(struct odTrivialArray* array, const void* extend_src, int32_t extend_count, int32_t stride);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odTrivialArray_pop(struct odTrivialArray* array, int32_t pop_count, int32_t stride);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odTrivialArray_swap_pop(struct odTrivialArray* array, int32_t i, int32_t stride);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odTrivialArray_assign(struct odTrivialArray* array, const void* assign_src, int32_t assign_count, int32_t stride);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD void*
odTrivialArray_get(struct odTrivialArray* array, int32_t i, int32_t stride);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const void*
odTrivialArray_get_const(const struct odTrivialArray* array, int32_t i, int32_t stride);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD void*
odTrivialArray_begin(struct odTrivialArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const void*
odTrivialArray_begin_const(const struct odTrivialArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD void*
odTrivialArray_end(struct odTrivialArray* array, int32_t stride);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const void*
odTrivialArray_end_const(const struct odTrivialArray* array, int32_t stride);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odType*
odArray_get_type_constructor(void);
OD_API_C OD_CORE_MODULE void
odArray_swap(struct odArray* array1, struct odArray* array2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odArray_check_valid(const struct odArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odArray_get_debug_string(const struct odArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odArray_init(struct odArray* array, const struct odType* type);
OD_API_C OD_CORE_MODULE void
odArray_destroy(struct odArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odType*
odArray_get_type(const struct odArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD int32_t
odArray_get_capacity(const struct odArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odArray_set_capacity(struct odArray* array, int32_t new_capacity);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odArray_ensure_capacity(struct odArray* array, int32_t min_capacity);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD int32_t
odArray_get_count(const struct odArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odArray_set_count(struct odArray* array, int32_t new_count);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odArray_extend(struct odArray* array, void* moved_src, int32_t moved_count);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odArray_pop(struct odArray* array, int32_t pop_count);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odArray_swap_pop(struct odArray* array, int32_t i);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD void*
odArray_get(struct odArray* array, int32_t i);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const void*
odArray_get_const(const struct odArray* array, int32_t i);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD void*
odArray_begin(struct odArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const void*
odArray_begin_const(const struct odArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD void*
odArray_end(struct odArray* array);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const void*
odArray_end_const(const struct odArray* array);
