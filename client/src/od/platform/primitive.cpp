#include <od/platform/primitive.h>

#include <cmath>
#include <cstdlib>

#include <od/core/debug.h>
#include <od/platform/vertex.h>

static int odTriangleVertices_compare(const void* triangle1, const void* triangle2);

bool odRectPrimitive_check_valid(const odRectPrimitive* rect) {
	if (!OD_DEBUG_CHECK(rect != nullptr)) {
		return false;
	}

	odBounds floored_texture_bounds = rect->texture_bounds;
	odBounds_floor(&floored_texture_bounds);

	if (!OD_CHECK(std::isfinite(rect->depth))
		|| (!OD_CHECK(odBounds_check_valid(&rect->bounds)))
		|| (!OD_CHECK(odBounds_check_valid(&rect->texture_bounds)))
		|| (!OD_CHECK(odBounds_equals(&floored_texture_bounds, &rect->texture_bounds)))) {
		return false;
	}

	return true;
}
void odRectPrimitive_get_vertices(const odRectPrimitive* rect, odVertex *out_vertices) {
	if (!OD_DEBUG_CHECK(odRectPrimitive_check_valid(rect))
		|| (!OD_DEBUG_CHECK(out_vertices != nullptr))) {
		return;
	}

	float x1 = rect->bounds.x1;
	float x2 = rect->bounds.x2;
	float y1 = rect->bounds.y1;
	float y2 = rect->bounds.y2;
	float u1 = rect->texture_bounds.x1;
	float u2 = rect->texture_bounds.x2;
	float v1 = rect->texture_bounds.y1;
	float v2 = rect->texture_bounds.y2;
	odColor color = rect->color;
	float depth = rect->depth;

	odVertex top_left = odVertex{{x1, y1, depth, 1.0f}, color, u1, v1};
	odVertex top_right = odVertex{{x2, y1, depth, 1.0f}, color, u2, v1};
	odVertex bottom_left = odVertex{{x1, y2, depth, 1.0f}, color, u1, v2};
	odVertex bottom_right = odVertex{{x2, y2, depth, 1.0f}, color, u2, v2};

	// display rect as two triangles, with vertices in counter-clockwise (y up) order:
	int32_t i = 0;
	out_vertices[i++] = top_left;
	out_vertices[i++] = bottom_left;
	out_vertices[i++] = top_right;

	out_vertices[i++] = top_right;
	out_vertices[i++] = bottom_left;
	out_vertices[i++] = bottom_right;
}

static int odTriangleVertices_compare(const void* triangle1, const void* triangle2) {
	if (!OD_DEBUG_CHECK(triangle1 != nullptr)
		|| !OD_DEBUG_CHECK(triangle2 != nullptr)) {
		return false;
	}

	float z1 = static_cast<const odVertex*>(triangle1)->pos.vector[2];
	float z2 = static_cast<const odVertex*>(triangle2)->pos.vector[2];

	if (z1 < z2) {
		return -1;
	} else if (z1 > z2) {
		return 1;
	}

	return 0;
}
void odTriangleVertices_sort_triangles(odVertex* triangles, int32_t triangles_count) {
	if (!OD_DEBUG_CHECK((triangles_count == 0) || (triangles != nullptr))
		|| !OD_DEBUG_CHECK(triangles_count >= 0)) {
		return;
	}

	qsort(triangles, static_cast<size_t>(triangles_count), 3 * sizeof(odVertex), odTriangleVertices_compare);
}
