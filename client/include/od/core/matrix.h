#pragma once

#include <od/core/module.h>

#define OD_MATRIX4_ELEM_COUNT 16

struct odVector4f;

struct odMatrix4f {
	float matrix[OD_MATRIX4_ELEM_COUNT];  // column-major
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odMatrix4f_get_debug_string(const struct odMatrix4f* matrix);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odMatrix4f_check_valid(const struct odMatrix4f* matrix);
// ensures transforming a 3d vector preserves the form x,y,z,1
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odMatrix4f_check_valid_transform_3d(const struct odMatrix4f* matrix);
OD_API_C OD_CORE_MODULE void
odMatrix4f_init_transform_3d(struct odMatrix4f* matrix,
							float scale_x, float scale_y, float scale_z,
							float translate_x, float translate_y, float translate_z);
OD_API_C OD_CORE_MODULE void
odMatrix4f_init_view_2d(struct odMatrix4f* matrix, int32_t width, int32_t height);
OD_API_C OD_CORE_MODULE void
odMatrix4f_multiply(struct odMatrix4f* matrix, const struct odMatrix4f* other);
OD_API_C OD_CORE_MODULE void
odMatrix4f_multiply_vector_4d(const struct odMatrix4f* matrix, struct odVector4f* vector);
OD_API_C OD_CORE_MODULE void
odMatrix4f_multiply_vector_3d(const struct odMatrix4f* matrix, struct odVector4f* vector);
OD_API_C OD_CORE_MODULE void
odMatrix4f_scale_3d(struct odMatrix4f* matrix, float scale_x, float scale_y, float scale_z);
OD_API_C OD_CORE_MODULE void
odMatrix4f_translate_3d(struct odMatrix4f* matrix, float translate_x, float translate_y, float translate_z);
OD_API_C OD_CORE_MODULE void
odMatrix4f_rotate_clockwise_z(struct odMatrix4f* matrix, float angle_deg);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odMatrix4f_equals(const struct odMatrix4f* matrix1, const struct odMatrix4f* matrix2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odMatrix4f_epsilon_equals(const struct odMatrix4f* matrix1, const struct odMatrix4f* matrix2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odMatrix4f*
odMatrix4f_get_identity(void);
