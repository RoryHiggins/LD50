#include <od/platform/primitive.h>

#include <cmath>

#include <od/core/debug.h>
#include <od/platform/vertex.h>

bool odPrimitiveQuad_check_valid(const odPrimitiveQuad* quad) {
	if (!OD_DEBUG_CHECK(quad != nullptr)) {
		return false;
	}

	odBounds floored_texture_bounds = quad->texture_bounds;
	odBounds_floor(&floored_texture_bounds);

	if (!OD_CHECK(std::isfinite(quad->depth))
		|| (!OD_CHECK(odBounds_check_valid(&quad->bounds)))
		|| (!OD_CHECK(odBounds_check_valid(&quad->texture_bounds)))
		|| (!OD_CHECK(odBounds_equals(&floored_texture_bounds, &quad->texture_bounds)))) {
		return false;
	}

	return true;
}
bool odPrimitiveQuad_get_vertices(const odPrimitiveQuad* quad, odVertex *out_vertices) {
	if (!OD_DEBUG_CHECK(odPrimitiveQuad_check_valid(quad))
		|| (!OD_DEBUG_CHECK(out_vertices != nullptr))) {
		return false;
	}

	float x1 = quad->bounds.x1;
	float x2 = quad->bounds.x2;
	float y1 = quad->bounds.y1;
	float y2 = quad->bounds.y2;
	float u1 = quad->texture_bounds.x1;
	float u2 = quad->texture_bounds.x2;
	float v1 = quad->texture_bounds.y1;
	float v2 = quad->texture_bounds.y2;
	odColor color = quad->color;
	float depth = quad->depth;

	odVertex top_left = odVertex{{x1, y1, depth, 1.0f}, color, u1, v1};
	odVertex top_right = odVertex{{x2, y1, depth, 1.0f}, color, u2, v1};
	odVertex bottom_left = odVertex{{x1, y2, depth, 1.0f}, color, u1, v2};
	odVertex bottom_right = odVertex{{x2, y2, depth, 1.0f}, color, u2, v2};

	// display quad as two triangles, with vertices in counter-clockwise order:
	out_vertices[0] = top_left;
	out_vertices[1] = bottom_left;
	out_vertices[2] = top_right;

	out_vertices[3] = top_right;
	out_vertices[4] = bottom_left;
	out_vertices[5] = bottom_right;

	return true;
}
