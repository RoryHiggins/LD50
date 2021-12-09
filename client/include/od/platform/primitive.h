#pragma once

#include <od/platform/module.h>

#include <od/core/bounds.h>
#include <od/core/color.h>

#define OD_PRIMITIVE_RECT_VERTEX_COUNT 6

struct odVertex;

struct odPrimitiveRect {
	struct odBounds bounds;
	struct odBounds texture_bounds;
	struct odColor color;
	float depth;
};

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odPrimitiveRect_check_valid(const struct odPrimitiveRect* rect);
OD_API_C OD_PLATFORM_MODULE void
odPrimitiveRect_get_vertices(const struct odPrimitiveRect* rect, struct odVertex *out_vertices);
