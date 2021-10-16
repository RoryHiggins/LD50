#pragma once

#include <od/core/module.h>

struct odColor;

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

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
const char* odVertex_get_debug_string(const struct odVertex* vertex);

OD_API_C OD_ENGINE_CORE_MODULE
void odVertex_set_color(struct odVertex* vertex, const struct odColor* color);

OD_API_C OD_ENGINE_CORE_MODULE
void odVertex_get_color(const struct odVertex* vertex, struct odColor* out_color);

