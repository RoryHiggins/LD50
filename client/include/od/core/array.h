#pragma once

#include <od/core/module.h>

struct odType;
struct odArray;

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
const struct odType* odArray_get_type_constructor(void);

OD_API_C OD_ENGINE_CORE_MODULE
void odArray_swap(struct odArray* array1, struct odArray* array2);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
bool odArray_get_valid(const struct odArray* array);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
const char* odArray_get_debug_string(const struct odArray* array);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
const struct odType* odArray_get_type(const struct odArray* array);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
bool odArray_set_type(struct odArray* array, const struct odType* type);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
int32_t odArray_get_capacity(const struct odArray* array);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
bool odArray_set_capacity(struct odArray* array, int32_t new_capacity);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
bool odArray_ensure_capacity(struct odArray* array, int32_t min_capacity);

OD_API_C OD_ENGINE_CORE_MODULE
void odArray_release(struct odArray* array);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
int32_t odArray_get_count(const struct odArray* array);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
bool odArray_set_count(struct odArray* array, int32_t new_count);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
bool odArray_expand(struct odArray* array, void** out_expand_dest, int32_t expand_count);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
bool odArray_push(struct odArray* array, void* moved_src, int32_t moved_count);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
bool odArray_shrink(struct odArray* array, int32_t count);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
bool odArray_swap_pop(struct odArray* array, int32_t i);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
void* odArray_get(struct odArray* array, int32_t i);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
const void* odArray_get_const(const struct odArray* array, int32_t i);
