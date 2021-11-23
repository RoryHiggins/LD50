#pragma once

#include <od/core/module.h>

struct odVector;

struct odMatrix {
	float matrix[16];  // column-major
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odMatrix_get_debug_string(const struct odMatrix* matrix);
OD_API_C OD_CORE_MODULE void
odMatrix_init(struct odMatrix* out_matrix,
			  float scale_x, float scale_y, float scale_z,
			  float translate_x, float translate_y, float translate_z);
OD_API_C OD_CORE_MODULE void
odMatrix_init_view_2d(struct odMatrix* out_matrix, int32_t width, int32_t height);
OD_API_C OD_CORE_MODULE void
odMatrix_multiply(struct odMatrix* out_matrix, struct odMatrix* a, struct odMatrix* b);
OD_API_C OD_CORE_MODULE void
odMatrix_multiply_vector(struct odVector* out_vector, struct odVector* a, struct odMatrix* b);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odMatrix*
odMatrix_get_identity(void);
