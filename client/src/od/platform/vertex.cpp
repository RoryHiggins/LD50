#include <od/platform/vertex.h>

#include <od/core/debug.h>

const char* odVertex_get_debug_string(const odVertex* vertex) {
	if (vertex == nullptr) {
		return "odVertex{this=nullptr}";
	}

	return odDebugString_format(
		"odVertex{this=%p, pos=%s, col=%s, u=%f, v=%f}",
		static_cast<const void*>(vertex),
		odVector_get_debug_string(&vertex->pos),
		odColor_get_debug_string(&vertex->col),
		static_cast<double>(vertex->u),
		static_cast<double>(vertex->v));
}
