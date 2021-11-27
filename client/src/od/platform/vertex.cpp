#include <od/platform/vertex.h>

#include <cmath>

#include <od/core/debug.h>

const char* odVertex_get_debug_string(const odVertex* vertex) {
	if (vertex == nullptr) {
		return "odVertex{this=nullptr}";
	}

	return odDebugString_format(
		"odVertex{this=%p, pos=%s, col=%s, u=%g, v=%g}",
		static_cast<const void*>(vertex),
		odVector4_get_debug_string(&vertex->pos),
		odColor_get_debug_string(&vertex->col),
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
