#pragma once

#include <od/platform/module.h>

#include <od/core/bounds.h>
#include <od/core/color.h>

#define OD_PRIMITIVE_QUAD_VERTEX_COUNT 6

struct odVertex;

struct odPrimitiveQuad {
	struct odBounds bounds;
	struct odBounds texture_bounds;
	struct odColor color;
	float depth;
};

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odPrimitiveQuad_check_valid(const struct odPrimitiveQuad* quad);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odPrimitiveQuad_get_vertices(const struct odPrimitiveQuad* quad, struct odVertex *out_vertices);
