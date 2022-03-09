#pragma once

#include <od/core/module.h>

#define OD_MATRIX_ELEM_COUNT 16

struct odVector;

struct odMatrix {
	float matrix[OD_MATRIX_ELEM_COUNT];  // column-major
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odMatrix_get_debug_string(const struct odMatrix* matrix);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odMatrix_check_valid(const struct odMatrix* matrix);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odMatrix_check_valid_3d(const struct odMatrix* matrix);
OD_API_C OD_CORE_MODULE void
odMatrix_init_3d(struct odMatrix* matrix,
				 float scale_x, float scale_y, float scale_z,
				 float translate_x, float translate_y, float translate_z);
OD_API_C OD_CORE_MODULE void
odMatrix_init_ortho_2d(struct odMatrix* matrix, int32_t width, int32_t height);
OD_API_C OD_CORE_MODULE void
odMatrix_multiply(struct odMatrix* matrix, const struct odMatrix* other);
OD_API_C OD_CORE_MODULE void
odMatrix_multiply_vector(const struct odMatrix* matrix, struct odVector* vector);
OD_API_C OD_CORE_MODULE void
odMatrix_multiply_vector_3d(const struct odMatrix* matrix, struct odVector* vector);
OD_API_C OD_CORE_MODULE void
odMatrix_scale_3d(struct odMatrix* matrix, float scale_x, float scale_y, float scale_z);
OD_API_C OD_CORE_MODULE void
odMatrix_translate_3d(struct odMatrix* matrix, float translate_x, float translate_y, float translate_z);
OD_API_C OD_CORE_MODULE void
odMatrix_rotate_z_3d(struct odMatrix* matrix, float rotate_clock_deg);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odMatrix_get_equals(const struct odMatrix* matrix1, const struct odMatrix* matrix2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odMatrix_epsilon_get_equals(const struct odMatrix* matrix1, const struct odMatrix* matrix2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odMatrix*
odMatrix_get_identity(void);
