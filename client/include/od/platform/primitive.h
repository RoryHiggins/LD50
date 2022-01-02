#pragma once

#include <od/platform/module.h>

#include <od/core/bounds.h>
#include <od/core/color.h>
#include <od/core/vertex.h>

#define OD_SPRITE_VERTEX_COUNT 6

#define OD_TRIANGLE_VERTEX_COUNT 3

struct odSpritePrimitive {
	struct odBounds bounds;
	struct odBounds texture_bounds;
	struct odColor color;
	float depth;
};

struct odTrianglePrimitive {
	struct odVertex vertices[OD_TRIANGLE_VERTEX_COUNT];
};

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odSpritePrimitive_check_valid(const struct odSpritePrimitive* rect);
OD_API_C OD_PLATFORM_MODULE void
odSpritePrimitive_get_vertices(const struct odSpritePrimitive* rect, struct odVertex *out_vertices);

OD_API_C OD_PLATFORM_MODULE void
odTrianglePrimitive_sort_vertices(struct odTrianglePrimitive* triangles, int32_t triangles_count);
