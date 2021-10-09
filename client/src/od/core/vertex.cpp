#include <od/core/vertex.h>

#include <od/core/debug.h>
#include <od/core/color.h>

const char* odVertex_get_debug_string(const odVertex* vertex) {
	if (vertex == nullptr) {
		return "odVertex{this=nullptr}";
	}

	return odDebugString_format(
		"odVertex{this=%p, x=%f, y=%f, z=%f r=%d, g=%d, b=%d, a=%d, u=%f, v=%f}",
		static_cast<const void*>(vertex),
		static_cast<double>(vertex->z),
		static_cast<double>(vertex->y),
		static_cast<double>(vertex->z),
		static_cast<int>(vertex->r),
		static_cast<int>(vertex->g),
		static_cast<int>(vertex->b),
		static_cast<int>(vertex->a),
		static_cast<double>(vertex->u),
		static_cast<double>(vertex->v));
}
void odVertex_set_color(odVertex* vertex, const odColor* color) {
	if (vertex == nullptr) {
		OD_ERROR("vertex == nullptr");
		return;
	}

	if (color == nullptr) {
		OD_ERROR("color == nullptr");
		return;
	}

	vertex->r = color->r;
	vertex->g = color->g;
	vertex->b = color->b;
	vertex->a = color->a;
}
void odVertex_get_color(const odVertex* vertex, odColor* out_color) {
	if (vertex == nullptr) {
		OD_ERROR("vertex == nullptr");
		return;
	}

	if (out_color == nullptr) {
		OD_ERROR("out_color == nullptr");
		return;
	}

	out_color->r = vertex->r;
	out_color->g = vertex->g;
	out_color->b = vertex->b;
	out_color->a = vertex->a;
}
