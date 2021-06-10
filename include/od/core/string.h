#pragma once

#include <od/core/api.h>

struct odString;

OD_API_ENGINE_C const struct odType* odString_get_type_constructor(void);
OD_API_ENGINE_C bool odString_copy(struct odString* string, const struct odString* src_string);
OD_API_ENGINE_C void odString_swap(struct odString* string1, struct odString* string2);
OD_API_ENGINE_C const char* odString_get_debug_string(const struct odString* string);
OD_API_ENGINE_C void odString_release(struct odString* string);
OD_API_ENGINE_C bool odString_set_capacity(struct odString* string, uint32_t new_capacity);
OD_API_ENGINE_C uint32_t odString_get_capacity(const struct odString* string);
OD_API_ENGINE_C bool odString_ensure_capacity(struct odString* string, uint32_t min_capacity);
OD_API_ENGINE_C bool odString_set_count(struct odString* string, uint32_t new_count);
OD_API_ENGINE_C uint32_t odString_get_count(const struct odString* string);
OD_API_ENGINE_C bool odString_push(struct odString* string, const char* str, uint32_t str_count);
OD_API_ENGINE_C bool odString_push_formatted_variadic(struct odString* string, const char* format_c_str, va_list args);
OD_API_ENGINE_C bool odString_push_formatted(struct odString* string, const char* format_c_str, ...);
OD_API_ENGINE_C bool odString_ensure_null_terminated(struct odString* string);
OD_API_ENGINE_C bool odString_get_null_terminated(const struct odString* string);
OD_API_ENGINE_C char* odString_get(struct odString* string, uint32_t i);
OD_API_ENGINE_C const char* odString_get_const(const struct odString* string, uint32_t i);
