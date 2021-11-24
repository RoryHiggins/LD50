#include <od/core/bounds.h>

#include <od/core/debug.h>

const char* odBounds_get_debug_string(const odBounds* bounds) {
	if (bounds == nullptr) {
		return "odBounds{this=nullptr}";
	}

	return odDebugString_format(
		"odBounds{this=%p, x=%g, y=%g, width=%g, height=%g}",
		static_cast<const void*>(bounds),
		static_cast<double>(bounds->x),
		static_cast<double>(bounds->y),
		static_cast<double>(bounds->width),
		static_cast<double>(bounds->height));
}
bool odBounds_get_valid(const struct odBounds* bounds) {
	if (!OD_DEBUG_CHECK(bounds != nullptr)) {
		return false;
	}

	if ((bounds->width < 0)
		|| (bounds->height < 0)) {
		return false;
	}

	return true;
}
bool odBounds_collides(const odBounds* a, const odBounds* b) {
	if (!OD_DEBUG_CHECK(a != nullptr)
		|| !OD_DEBUG_CHECK(b != nullptr)) {
		return false;
	}

	return (
		(a->x < (b->x + b->width))
		&& ((a->x + a->width) > b->x)
		&& (a->y < (b->y + b->height))
		&& ((a->y + a->height) > b->y)
		&& (a->width > 0)
		&& (a->height > 0)
		&& (b->width > 0)
		&& (b->height > 0)
	);
}
bool odBounds_equals(const odBounds* a, const odBounds* b) {
	if (!OD_DEBUG_CHECK(a != nullptr)
		|| !OD_DEBUG_CHECK(b != nullptr)) {
		return false;
	}

	if ((a->x != b->x)
		|| (a->y != b->y)
		|| (a->width != b->width)
		|| (a->height != b->height)) {
		return false;
	}

	return true;
}
