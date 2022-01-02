#include <od/platform/primitive.h>

#include <cmath>
#include <cstdlib>

#include <od/core/debug.h>
#include <od/core/vertex.h>

static int odTrianglePrimitive_compare(const void* triangle1, const void* triangle2);

bool odSpritePrimitive_check_valid(const odSpritePrimitive* sprite) {
	if (!OD_DEBUG_CHECK(sprite != nullptr)) {
		return false;
	}

	if (!OD_CHECK(std::isfinite(sprite->depth))
		|| (!OD_CHECK(odBounds_check_valid(&sprite->bounds)))
		|| (!OD_CHECK(odBounds_check_valid(&sprite->texture_bounds)))) {
		return false;
	}

	return true;
}
void odSpritePrimitive_get_vertices(const odSpritePrimitive* sprite, odVertex *out_vertices) {
	if (!OD_DEBUG_CHECK(odSpritePrimitive_check_valid(sprite))
		|| !OD_DEBUG_CHECK(out_vertices != nullptr)
		|| !OD_DEBUG_CHECK(odBounds_fits_float(&sprite->bounds))
		|| !OD_DEBUG_CHECK(odBounds_fits_float(&sprite->texture_bounds))) {
		return;
	}

	float x1 = static_cast<float>(sprite->bounds.x1);
	float x2 = static_cast<float>(sprite->bounds.x2);
	float y1 = static_cast<float>(sprite->bounds.y1);
	float y2 = static_cast<float>(sprite->bounds.y2);
	float u1 = static_cast<float>(sprite->texture_bounds.x1);
	float u2 = static_cast<float>(sprite->texture_bounds.x2);
	float v1 = static_cast<float>(sprite->texture_bounds.y1);
	float v2 = static_cast<float>(sprite->texture_bounds.y2);

	odVertex top_left = odVertex{odVector{x1, y1, sprite->depth, 1.0f}, sprite->color, u1, v1};
	odVertex top_right = odVertex{odVector{x2, y1, sprite->depth, 1.0f}, sprite->color, u2, v1};
	odVertex bottom_left = odVertex{odVector{x1, y2, sprite->depth, 1.0f}, sprite->color, u1, v2};
	odVertex bottom_right = odVertex{odVector{x2, y2, sprite->depth, 1.0f}, sprite->color, u2, v2};

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

	float z1 = static_cast<const odTrianglePrimitive*>(triangle1)->vertices[0].pos.z;
	float z2 = static_cast<const odTrianglePrimitive*>(triangle2)->vertices[0].pos.z;

	if (z1 < z2) {
		return -1;
	} else if (z1 > z2) {
		return 1;
	}

	return 0;
}
void odTrianglePrimitive_sort_vertices(odTrianglePrimitive* triangles, int32_t triangles_count) {
	if (!OD_DEBUG_CHECK((triangles_count == 0) || (triangles != nullptr))
		|| !OD_DEBUG_CHECK(triangles_count >= 0)) {
		return;
	}

	qsort(
		triangles,
		static_cast<size_t>(triangles_count),
		sizeof(odTrianglePrimitive),
		odTrianglePrimitive_compare);
}
