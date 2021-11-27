#pragma once

#include <od/core/module.h>

#define OD_MATRIX4_ELEM_COUNT 16

struct odVector4;

struct odMatrix4 {
	float matrix[OD_MATRIX4_ELEM_COUNT];  // column-major
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odMatrix4_get_debug_string(const struct odMatrix4* matrix);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odMatrix4_check_valid(const struct odMatrix4* matrix);
OD_API_C OD_CORE_MODULE void
odMatrix4_init(struct odMatrix4* out_matrix,
			  float scale_x, float scale_y, float scale_z,
			  float translate_x, float translate_y, float translate_z);
OD_API_C OD_CORE_MODULE void
odMatrix4_init_view_2d(struct odMatrix4* out_matrix, int32_t width, int32_t height);
OD_API_C OD_CORE_MODULE void
odMatrix4_multiply(struct odMatrix4* out_matrix, struct odMatrix4* a, struct odMatrix4* b);
OD_API_C OD_CORE_MODULE void
odMatrix4_multiply_vector(struct odVector4* out_vector, struct odVector4* a, struct odMatrix4* b);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odMatrix4*
odMatrix4_get_identity(void);
