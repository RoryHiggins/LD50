#pragma once

#include <od/core/module.h>

struct odTransform {
	float matrix[16];
};

OD_API_C OD_ENGINE_CORE_MODULE OD_NO_DISCARD
struct odTransform odTransform_create(
	float scale_x,
	float scale_y,
	float scale_z,
	float translate_x,
	float translate_y,
	float translate_z);

OD_API_C OD_ENGINE_CORE_MODULE OD_NO_DISCARD
struct odTransform odTransform_create_view_transform(int32_t width, int32_t height);

static const struct odTransform odTransform_identity = {{
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
}};
