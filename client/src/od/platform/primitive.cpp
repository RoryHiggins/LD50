#include <od/platform/primitive.h>

#include <cmath>

#include <algorithm>

#include <od/core/debug.h>
#include <od/core/bounds.h>
#include <od/core/vertex.h>

static bool odTrianglePrimitive_compare(const odTrianglePrimitive& triangle1, const odTrianglePrimitive& triangle2);

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
		|| !OD_DEBUG_CHECK(out_vertices != nullptr)) {
		return;
	}

	float x1 = sprite->bounds.x1;
	float x2 = sprite->bounds.x2;
	float y1 = sprite->bounds.y1;
	float y2 = sprite->bounds.y2;

	float u1 = sprite->texture_bounds.x1;
	float u2 = sprite->texture_bounds.x2;
	float v1 = sprite->texture_bounds.y1;
	float v2 = sprite->texture_bounds.y2;

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


bool odLinePrimitive_check_valid(const odLinePrimitive* line) {
	if (!OD_DEBUG_CHECK(line != nullptr)
		|| !OD_CHECK(std::isfinite(line->depth))
		|| !OD_CHECK(odBounds_check_valid(&line->bounds))) {
		return false;
	}

	return true;
}
void odLinePrimitive_get_vertices(const odLinePrimitive* line, odVertex *out_vertices) {
	if (!OD_DEBUG_CHECK(odLinePrimitive_check_valid(line))
		|| !OD_DEBUG_CHECK(out_vertices != nullptr)) {
		return;
	}

	odVertex start = odVertex{odVector{line->bounds.x1, line->bounds.y1, line->depth, 1.0f}, line->color, 0.0f, 0.0f};
	odVertex end = odVertex{odVector{line->bounds.x2, line->bounds.y2, line->depth, 1.0f}, line->color, 0.0f, 0.0f};

	out_vertices[0] = start;
	out_vertices[1] = start;
	out_vertices[2] = end;
	out_vertices[3] = end;
	out_vertices[4] = start;
	out_vertices[5] = end;

	// Give the triangles actual width -  OpenGL can't render degenerate triangles
	static const float width = 1.0f;
	float length_x = fabsf(end.pos.x - start.pos.x);
	float length_y = fabsf(end.pos.y - start.pos.y);
	float is_horizontal_line = width * static_cast<float>(length_x > length_y);
	float is_vertical_line = width * static_cast<float>(length_x <= length_y);
	out_vertices[1].pos.x += is_vertical_line;
	out_vertices[1].pos.y += is_horizontal_line;
	out_vertices[4].pos.x += is_vertical_line;
	out_vertices[4].pos.y += is_horizontal_line;
	out_vertices[5].pos.x += is_vertical_line;
	out_vertices[5].pos.y += is_horizontal_line;
}

static bool odTrianglePrimitive_compare(const odTrianglePrimitive& triangle1, const odTrianglePrimitive& triangle2) {
	if (triangle1.vertices[0].pos.z > triangle2.vertices[0].pos.z) {
		return true;
	}
	
	return false;
}
void odTrianglePrimitive_sort_triangles(odTrianglePrimitive* triangles, int32_t triangles_count) {
	if (!OD_DEBUG_CHECK((triangles_count == 0) || (triangles != nullptr))
		|| !OD_DEBUG_CHECK(triangles_count >= 0)) {
		return;
	}

	std::stable_sort(triangles, triangles + triangles_count, odTrianglePrimitive_compare);
}
void odTrianglePrimitive_sort_vertices(odVertex* vertices, int32_t vertices_count) {
	odTrianglePrimitive_sort_triangles(
		reinterpret_cast<odTrianglePrimitive*>(vertices),
		vertices_count / 3
	);
}
