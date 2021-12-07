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
	if (!OD_CHECK(odPrimitiveQuad_check_valid(quad))
		|| (!OD_CHECK(out_vertices != nullptr))) {
		return false;
	}

	float x1 = quad->bounds.x;
	float y1 = quad->bounds.y;
	float x2 = x1 + quad->bounds.width;
	float y2 = y1 + quad->bounds.height;
	float u1 = quad->texture_bounds.x;
	float v1 = quad->texture_bounds.y;
	float u2 = u1 + quad->texture_bounds.width;
	float v2 = v1 + quad->texture_bounds.height;
	odColor color = quad->color;
	float depth = quad->depth;

	int32_t i = 0;
	out_vertices[i++] = odVertex{{x1, y1, depth, 1.0f}, color, u1, v1};
	out_vertices[i++] = odVertex{{x1, y2, depth, 1.0f}, color, u1, v2};
	out_vertices[i++] = odVertex{{x2, y1, depth, 1.0f}, color, u2, v1};

	out_vertices[i++] = odVertex{{x2, y1, depth, 1.0f}, color, u2, v1};
	out_vertices[i++] = odVertex{{x1, y2, depth, 1.0f}, color, u1, v2};
	out_vertices[i++] = odVertex{{x2, y2, depth, 1.0f}, color, u2, v2};

	return true;
}
