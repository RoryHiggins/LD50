#include <od/core/vertex.hpp>

#include <cmath>

#include <od/core/debug.h>
#include <od/core/matrix.h>

const char* odVertex_get_debug_string(const odVertex* vertex) {
	if (vertex == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"pos\": %s, \"color\": %s, \"u\": %g, \"v\": %g}",
		odVector4f_get_debug_string(&vertex->pos),
		odColorRGBA32_get_debug_string(&vertex->color),
		static_cast<double>(vertex->u),
		static_cast<double>(vertex->v));
}
bool odVertex_check_valid(const odVertex* vertex) {
	if (!OD_CHECK(vertex != nullptr)
		|| !OD_CHECK(odVector4f_check_valid(&vertex->pos))
		|| !OD_CHECK(std::isfinite(vertex->u) || (vertex->u < 0) || (vertex->u > 65535.0f))
		|| !OD_CHECK(std::isfinite(vertex->v) || (vertex->v < 0) || (vertex->v > 65535.0f))) {
		return false;
	}

	return true;
}
bool odVertex_check_valid_batch(const odVertex* vertices, int32_t vertices_count) {
	if (!OD_CHECK((vertices_count == 0) || (vertices != nullptr))
		|| !OD_CHECK(vertices_count >= 0)) {
		return false;
	}

	for (int32_t i = 0; i < vertices_count; i++) {
		if (!OD_CHECK(odVertex_check_valid(vertices + i))) {
			return false;
		}
	}

	return true;
}
void odVertex_transform(odVertex* vertex, const odMatrix4f* matrix) {
	if (!OD_DEBUG_CHECK(vertex != nullptr)
		|| !OD_DEBUG_CHECK(odMatrix4f_check_valid(matrix))) {
		return;
	}

	odMatrix4f_multiply_vector(matrix, &vertex->pos);
}
void odVertex_transform_batch(odVertex* vertices, int32_t vertices_count, const odMatrix4f* matrix) {
	if (!OD_DEBUG_CHECK(odVertex_check_valid_batch(vertices, vertices_count))
		|| !OD_DEBUG_CHECK(vertices_count >= 0)) {
		return;
	}

	for (int32_t i = 0; i < vertices_count; i++) {
		odVertex_transform(vertices + i, matrix);
	}
}

template struct odTrivialArrayT<odVertex>;
