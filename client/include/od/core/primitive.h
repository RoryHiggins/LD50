#pragma once

#include <od/core/module.h>

struct odBounds {
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
};

struct odColor {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct odVertex {
	float x;
	float y;
	float z;

	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

	float u;
	float v;
};

struct odVector {
	float vector[4];
};

struct odTransform {
	float matrix[16];  // column-major memory layout
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const char* odVertex_get_debug_string(const struct odVertex* vertex);

OD_API_C OD_CORE_MODULE
void odVertex_set_color(struct odVertex* vertex, const struct odColor* color);

OD_API_C OD_CORE_MODULE
void odVertex_get_color(const struct odVertex* vertex, struct odColor* out_color);


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
