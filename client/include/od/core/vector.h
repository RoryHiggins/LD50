#pragma once

#include <od/core/module.h>

#define OD_VECTOR4_ELEM_COUNT 4

struct odVector4 {
	float vector[OD_VECTOR4_ELEM_COUNT];  // xyzw
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVector4_check_valid(const struct odVector4* vector);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odVector4_get_debug_string(const struct odVector4* vector);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVector4_equals(const struct odVector4* vector1, const struct odVector4* vector2);
