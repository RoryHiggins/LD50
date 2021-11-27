#include <od/core/bounds.h>

#include <cmath>

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
bool odBounds_check_valid(const struct odBounds* bounds) {
	if (!OD_CHECK(bounds != nullptr)
		|| !OD_CHECK(std::isfinite(bounds->x))
		|| !OD_CHECK(std::isfinite(bounds->y))
		|| !OD_CHECK(std::isfinite(bounds->width))
		|| !OD_CHECK(std::isfinite(bounds->height))
		|| !OD_CHECK(bounds->width >= 0)
		|| !OD_CHECK(bounds->height >= 0)) {
		return false;
	}

	return true;
}
bool odBounds_collides(const odBounds* a, const odBounds* b) {
	if (!OD_DEBUG_CHECK(odBounds_check_valid(a))
		|| !OD_DEBUG_CHECK(odBounds_check_valid(b))) {
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
void odBounds_floor(odBounds* bounds) {
	if (!OD_DEBUG_CHECK(bounds != nullptr)) {
		return;
	}

	bounds->x = floorf(bounds->x);
	bounds->y = floorf(bounds->y);
	bounds->width = floorf(bounds->width);
	bounds->height = floorf(bounds->height);
}
