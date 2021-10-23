#pragma once

#include <od/core/module.h>

struct odColor;

struct odVertex {
	int32_t x;
	int32_t y;
	int32_t z;

	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

	int32_t u;
	int32_t v;
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const char* odVertex_get_debug_string(const struct odVertex* vertex);

OD_API_C OD_CORE_MODULE
void odVertex_set_color(struct odVertex* vertex, const struct odColor* color);

OD_API_C OD_CORE_MODULE
void odVertex_get_color(const struct odVertex* vertex, struct odColor* out_color);

