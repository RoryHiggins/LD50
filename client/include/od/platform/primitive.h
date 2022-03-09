#pragma once

#include <od/platform/module.h>

#include <od/core/bounds.h>
#include <od/core/color.h>
#include <od/core/vertex.h>

#define OD_TRIANGLE_VERTEX_COUNT 3
#define OD_QUAD_VERTEX_COUNT (2 * OD_TRIANGLE_VERTEX_COUNT)
#define OD_SPRITE_VERTEX_COUNT OD_QUAD_VERTEX_COUNT
#define OD_LINE_VERTEX_COUNT OD_QUAD_VERTEX_COUNT

struct odSpritePrimitive {
	struct odBounds bounds;
	struct odBounds texture_bounds;
	struct odColor color;
	float depth;
};

struct odLinePrimitive {
	struct odBounds bounds;
	struct odColor color;
	float depth;
};

struct odTrianglePrimitive {
	struct odVertex vertices[OD_TRIANGLE_VERTEX_COUNT];
};

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odSpritePrimitive_check_valid(const struct odSpritePrimitive* sprite);
OD_API_C OD_PLATFORM_MODULE void
odSpritePrimitive_get_vertices(const struct odSpritePrimitive* sprite, struct odVertex *out_vertices);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odLinePrimitive_check_valid(const struct odLinePrimitive* line);
OD_API_C OD_PLATFORM_MODULE void
odLinePrimitive_get_vertices(const struct odLinePrimitive* line, struct odVertex *out_vertices);

OD_API_C OD_PLATFORM_MODULE void
odTrianglePrimitive_sort_triangles(struct odTrianglePrimitive* triangles, int32_t triangles_count);
OD_API_C OD_PLATFORM_MODULE void
odTrianglePrimitive_sort_vertices(struct odVertex* vertices, int32_t vertices_count);
