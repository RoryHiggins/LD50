#pragma once

#include <od/core/module.h>
#include <od/core/math.h>

#define OD_VECTOR4_ELEM_COUNT 4

struct odVector4f {
	float vector[OD_VECTOR4_ELEM_COUNT];  // x,y,z,w
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVector4f_check_valid(const struct odVector4f* vector);
// ensures the form x,y,z,1
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVector4f_check_valid_3d(const struct odVector4f* vector3);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVector4f_check_valid_2d(const struct odVector4f* vector3);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odVector4f_get_debug_string(const struct odVector4f* vector);
OD_API_C OD_CORE_MODULE void
odVector4f_init_3d(struct odVector4f* vector, float x, float y, float z);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVector4f_equals(const struct odVector4f* vector1, const struct odVector4f* vector2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVector4f_epsilon_equals(const struct odVector4f* vector1, const struct odVector4f* vector2);
