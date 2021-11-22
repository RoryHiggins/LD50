#pragma once

#include <od/core/module.h>

struct odBounds {
	float x;
	float y;
	float width;
	float height;
};
struct odVector {
	float vector[4];  // xyzw
};
struct odTransform {
	float matrix[16];  // column-major
};
struct odColor {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};
struct odVertex {
	odVector pos;
	odColor col;
	float u;
	float v;
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odBounds_get_debug_string(const struct odBounds* bounds);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odVector_get_debug_string(const struct odVector* vector);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odTransform_get_debug_string(const struct odTransform* transform);

OD_API_C OD_CORE_MODULE void
odTransform_init(struct odTransform* out_transform,
				 float scale_x, float scale_y, float scale_z,
				 float translate_x, float translate_y, float translate_z);
OD_API_C OD_CORE_MODULE void
odTransform_init_view_transform(struct odTransform* out_transform, int32_t width, int32_t height);
OD_API_C OD_CORE_MODULE void
odTransform_multiply(struct odTransform* out_transform, struct odTransform* a, struct odTransform* b);
OD_API_C OD_CORE_MODULE void
odTransform_multiply_vector(struct odVector* out_vector, struct odVector* a, struct odTransform* b);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odColor_get_debug_string(const struct odColor* color);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odVertex_get_debug_string(const struct odVertex* vertex);

static const struct odColor odColor_white = {0xFF, 0xFF, 0xFF, 0xFF};
static const struct odColor odColor_black = {0x00, 0x00, 0x00, 0xFF};
static const struct odColor odColor_red = {0xFF, 0x00, 0x00, 0xFF};
static const struct odColor odColor_green = {0x00, 0xFF, 0x00, 0xFF};
static const struct odColor odColor_blue = {0x00, 0x00, 0xFF, 0xFF};
static const struct odColor odColor_transparent = {0x00, 0x00, 0x00, 0x00};

static const struct odTransform odTransform_identity = {{
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
}};
