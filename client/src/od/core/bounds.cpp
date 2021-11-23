#include <od/core/bounds.h>

#include <od/core/debug.h>

const char* odBounds_get_debug_string(const odBounds* bounds) {
	if (bounds == nullptr) {
		return "odBounds{this=nullptr}";
	}

	return odDebugString_format(
		"odBounds{this=%p, x=%f, y=%f, width=%f, height=%f}",
		static_cast<const void*>(bounds),
		static_cast<double>(bounds->x),
		static_cast<double>(bounds->y),
		static_cast<double>(bounds->width),
		static_cast<double>(bounds->height));
}
