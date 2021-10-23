#pragma once

#include <od/core/module.h>

struct odVector {
	float vector[4];
};

struct odTransform {
	float matrix[16];  // column-major memory layout
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
struct odTransform odTransform_create(
	float scale_x,
	float scale_y,
	float scale_z,
	float translate_x,
	float translate_y,
	float translate_z);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
struct odTransform odTransform_create_view_transform(int32_t width, int32_t height);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
struct odTransform odTransform_multiply(struct odTransform a, struct odTransform b);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
struct odVector odTransform_multiply_vector(struct odVector a, struct odTransform b);

static const struct odTransform odTransform_identity = {{
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
}};
