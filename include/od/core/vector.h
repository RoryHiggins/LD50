#pragma once

#include <od/core/api.h>

struct odType;
struct odVector;

OD_API_C const struct odType* odVector_get_type_constructor(void);
OD_API_C void odVector_swap(struct odVector* vector1, struct odVector* vector2);
OD_API_C const char* odVector_get_debug_string(const struct odVector* vector);
OD_API_C const struct odType* odVector_get_type(const struct odVector* vector);
OD_API_C void odVector_set_type(struct odVector* vector, const struct odType* type);
OD_API_C uint32_t odVector_get_capacity(const struct odVector* vector);
OD_API_C bool odVector_set_capacity(struct odVector* vector, uint32_t new_capacity);
OD_API_C bool odVector_ensure_capacity(struct odVector* vector, uint32_t min_capacity);
OD_API_C void odVector_release(struct odVector* vector);
OD_API_C uint32_t odVector_get_count(const struct odVector* vector);
OD_API_C bool odVector_set_count(struct odVector* vector, uint32_t new_count);
OD_API_C bool odVector_push(struct odVector* vector, void* moved_src, uint32_t moved_count);
OD_API_C bool odVector_pop(struct odVector* vector, uint32_t count);
OD_API_C bool odVector_swap_pop(struct odVector* vector, uint32_t i);
OD_API_C void* odVector_get(struct odVector* vector, uint32_t i);
OD_API_C const void* odVector_get_const(const struct odVector* vector, uint32_t i);
