#include <od/core/vertex.hpp>

#include <cmath>

#include <od/core/debug.h>
#include <od/core/matrix.h>

template struct odFastArrayT<odVertex>;

const char* odVertex_get_debug_string(const odVertex* vertex) {
	if (vertex == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"pos\": %s, \"color\": %s, \"u\": %g, \"v\": %g}",
		odVector4_get_debug_string(&vertex->pos),
		odColor_get_debug_string(&vertex->color),
		static_cast<double>(vertex->u),
		static_cast<double>(vertex->v));
}
bool odVertex_check_valid(const odVertex* vertex) {
	if (!OD_CHECK(vertex != nullptr)
		|| !OD_CHECK(odVector4_check_valid(&vertex->pos))
		|| !OD_CHECK(std::isfinite(vertex->u) || (vertex->u < 0) || (vertex->u > 65535.0f))
		|| !OD_CHECK(std::isfinite(vertex->v) || (vertex->v < 0) || (vertex->v > 65535.0f))) {
		return false;
	}

	return true;
}
void odVertex_transform(odVertex* vertex, const struct odMatrix4* matrix) {
	if (!OD_DEBUG_CHECK(vertex != nullptr)
		|| !OD_DEBUG_CHECK(odMatrix4_check_valid(matrix))) {
		return;
	}

	odMatrix4_multiply_vector(matrix, &vertex->pos);
}