#pragma once

#include <od/platform/module.h>

#include <od/core/color.h>
#include <od/core/bounds.h>

struct odColor;

struct odAsciiTextPrimitive {
	const char* str;
	int32_t str_count;

	struct odBounds max_bounds;
	struct odColor color;
	float depth;
};

struct odAsciiFont {
	struct odBounds texture_bounds;
	int32_t char_width;
	int32_t char_height;
	char char_first;
	char char_last;
};

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAsciiTextPrimitive_check_valid(const struct odAsciiTextPrimitive* text);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD int32_t
odAsciiTextPrimitive_get_max_vertices_count(const struct odAsciiTextPrimitive* text);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAsciiFont_check_valid(const struct odAsciiFont* font);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAsciiFont_text_get_vertices(const struct odAsciiFont* font, const struct odAsciiTextPrimitive* text,
							  int32_t* opt_out_vertices_count, struct odBounds* opt_out_bounds,
							  struct odVertex* opt_out_vertices);
