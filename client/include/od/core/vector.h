#pragma once

#include <od/core/module.h>
#include <od/core/math.h>

#define OD_VECTOR4_ELEM_COUNT 4

struct odVector {
	float vector[OD_VECTOR4_ELEM_COUNT];  // x,y,z,w
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVector_check_valid(const struct odVector* vector);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odVector_get_debug_string(const struct odVector* vector);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVector_equals(const struct odVector* vector1, const struct odVector* vector2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVector_epsilon_equals(const struct odVector* vector1, const struct odVector* vector2);
