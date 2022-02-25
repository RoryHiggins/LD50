#include <od/core/vertex.hpp>

#include <cmath>

#include <od/core/debug.h>
#include <od/core/math.h>
#include <od/core/matrix.h>

const char* odVertex_get_debug_string(const odVertex* vertex) {
	if (vertex == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"pos\": %s, \"color\": %s, \"u\": %g, \"v\": %g}",
		odVector_get_debug_string(&vertex->pos),
		odColor_get_debug_string(&vertex->color),
		static_cast<double>(vertex->u),
		static_cast<double>(vertex->v));
}
bool odVertex_check_valid_3d(const odVertex* vertex) {
	if (!OD_CHECK(vertex != nullptr)
		|| !OD_CHECK(odVector_check_valid(&vertex->pos))
		|| !OD_CHECK(odFloat_is_precise_int(vertex->u))
		|| !OD_CHECK(vertex->u >= 0)
		|| !OD_CHECK(odFloat_is_precise_int(vertex->v))
		|| !OD_CHECK(vertex->v >= 0)) {
		return false;
	}

	return true;
}
bool odVertex_check_valid_batch_3d(const odVertex* vertices, int32_t vertices_count) {
	if (!OD_CHECK((vertices_count == 0) || (vertices != nullptr))
		|| !OD_CHECK(vertices_count >= 0)) {
		return false;
	}

	for (int32_t i = 0; i < vertices_count; i++) {
		if (!OD_CHECK(odVertex_check_valid_3d(vertices + i))) {
			return false;
		}
	}

	return true;
}
void odVertex_transform_3d(odVertex* vertex, const odMatrix* matrix) {
	if (!OD_DEBUG_CHECK(odVertex_check_valid_3d(vertex))
		|| !OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix))) {
		return;
	}

	odMatrix_multiply_vector_3d(matrix, &vertex->pos);

	OD_DISCARD(OD_DEBUG_CHECK(odVertex_check_valid_3d(vertex)));
}
void odVertex_transform_batch_3d(odVertex* vertices, int32_t vertices_count, const odMatrix* matrix) {
	if (!OD_DEBUG_CHECK(odVertex_check_valid_batch_3d(vertices, vertices_count))
		|| !OD_DEBUG_CHECK(vertices_count >= 0)
		|| !OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix))) {
		return;
	}

	for (int32_t i = 0; i < vertices_count; i++) {
		odVertex_transform_3d(vertices + i, matrix);
	}
}
