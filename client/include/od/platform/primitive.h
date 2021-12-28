#pragma once

#include <od/platform/module.h>

#include <od/core/bounds.h>
#include <od/core/color.h>

#define OD_RECT_PRIMITIVE_VERTEX_COUNT 6

struct odVertex;

struct odRectPrimitive {
	struct odBounds2 bounds;
	struct odBounds2 texture_bounds;
	struct odColor color;
	float depth;
};
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odRectPrimitive_check_valid(const struct odRectPrimitive* rect);
OD_API_C OD_PLATFORM_MODULE void
odRectPrimitive_get_vertices(const struct odRectPrimitive* rect, struct odVertex *out_vertices);

OD_API_C OD_PLATFORM_MODULE void
odTriangleVertices_sort_triangles(struct odVertex* triangles, int32_t triangles_count);
