#pragma once

#include <od/core/module.h>

struct odString;

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odString_check_valid(const struct odString* string);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odString_get_debug_string(const struct odString* string);
OD_API_C OD_CORE_MODULE void
odString_init(struct odString* string);
OD_API_C OD_CORE_MODULE void
odString_destroy(struct odString* string);
OD_API_C OD_CORE_MODULE void
odString_swap(struct odString* string1, struct odString* string2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odString_assign(struct odString* string, const char* assign_src, int32_t assign_count);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD int32_t
odString_compare(const struct odString* string1, const struct odString* string2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD int32_t
odString_get_capacity(const struct odString* string);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odString_set_capacity(struct odString* string, int32_t new_capacity);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odString_ensure_capacity(struct odString* string, int32_t min_capacity);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD int32_t
odString_get_count(const struct odString* string);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odString_set_count(struct odString* string, int32_t new_count);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odString_ensure_count(struct odString* string, int32_t min_count);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odString_extend(struct odString* string, const char* extend_src, int32_t extend_count);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odString_extend_formatted_variadic(struct odString* string, const char* format_c_str, va_list args);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odString_extend_formatted(struct odString* string, const char* format_c_str, ...);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odString_pop(struct odString* string, int32_t pop_count);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odString_swap_pop(struct odString* string, int32_t i);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odString_get_c_str(const struct odString* string);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD char*
odString_get(struct odString* string, int32_t i);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odString_get_const(const struct odString* string, int32_t i);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD char*
odString_begin(struct odString* string);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odString_begin_const(const struct odString* string);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD char*
odString_end(struct odString* string);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odString_end_const(const struct odString* string);
