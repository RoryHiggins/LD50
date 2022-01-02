#include <od/platform/primitive.h>

#include <cmath>
#include <cstdlib>

#include <od/core/debug.h>
#include <od/core/vertex.h>

static int odTrianglePrimitive_compare(const void* triangle1, const void* triangle2);

bool odRectPrimitive_check_valid(const odRectPrimitive* rect) {
	if (!OD_DEBUG_CHECK(rect != nullptr)) {
		return false;
	}

	if (!OD_CHECK(std::isfinite(rect->depth))
		|| (!OD_CHECK(odBounds_check_valid(&rect->bounds)))
		|| (!OD_CHECK(odBounds_check_valid(&rect->texture_bounds)))) {
		return false;
	}

	return true;
}
void odRectPrimitive_get_vertices(const odRectPrimitive* rect, odVertex *out_vertices) {
	if (!OD_DEBUG_CHECK(odRectPrimitive_check_valid(rect))
		|| !OD_DEBUG_CHECK(out_vertices != nullptr)
		|| !OD_DEBUG_CHECK(odBounds_fits_float(&rect->bounds))
		|| !OD_DEBUG_CHECK(odBounds_fits_float(&rect->texture_bounds))) {
		return;
	}

	float x1 = static_cast<float>(rect->bounds.x1);
	float x2 = static_cast<float>(rect->bounds.x2);
	float y1 = static_cast<float>(rect->bounds.y1);
	float y2 = static_cast<float>(rect->bounds.y2);
	float u1 = static_cast<float>(rect->texture_bounds.x1);
	float u2 = static_cast<float>(rect->texture_bounds.x2);
	float v1 = static_cast<float>(rect->texture_bounds.y1);
	float v2 = static_cast<float>(rect->texture_bounds.y2);

	odVertex top_left = odVertex{{x1, y1, rect->depth, 0.0f}, rect->color, u1, v1};
	odVertex top_right = odVertex{{x2, y1, rect->depth, 0.0f}, rect->color, u2, v1};
	odVertex bottom_left = odVertex{{x1, y2, rect->depth, 0.0f}, rect->color, u1, v2};
	odVertex bottom_right = odVertex{{x2, y2, rect->depth, 0.0f}, rect->color, u2, v2};

	// display as two triangles, vertices in counter-clockwise order, positive y as up:
	out_vertices[0] = top_left;
	out_vertices[1] = bottom_left;
	out_vertices[2] = top_right;
	out_vertices[3] = top_right;
	out_vertices[4] = bottom_left;
	out_vertices[5] = bottom_right;
}

static int odTrianglePrimitive_compare(const void* triangle1, const void* triangle2) {
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
void odTrianglePrimitive_sort_vertices(odVertex* triangles, int32_t triangles_count) {
	if (!OD_DEBUG_CHECK((triangles_count == 0) || (triangles != nullptr))
		|| !OD_DEBUG_CHECK(triangles_count >= 0)) {
		return;
	}

	qsort(triangles, static_cast<size_t>(triangles_count), 3 * sizeof(odVertex), odTrianglePrimitive_compare);
}
