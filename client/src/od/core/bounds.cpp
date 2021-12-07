#include <od/core/bounds.h>

#include <cmath>

#include <od/core/debug.h>

static bool odBounds_check_valid_coord(float x);

bool odBounds_check_valid_coord(float x) {
	const float max = (1 << 24);
	const float min = -max;

	if (!OD_CHECK(std::isfinite(x))
		|| !OD_CHECK((x >= min) && (x <= max))) {
		return false;
	}
	return true;
}
const char* odBounds_get_debug_string(const odBounds* bounds) {
	if (bounds == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"x1\": %g, \"x2\": %g, \"y1\": %g, \"y2\": %g}",
		static_cast<double>(bounds->x1),
		static_cast<double>(bounds->x2),
		static_cast<double>(bounds->y1),
		static_cast<double>(bounds->y2));
}
bool odBounds_check_valid(const odBounds* bounds) {
	if (!OD_CHECK(bounds != nullptr)
		|| !OD_CHECK(odBounds_check_valid_coord(bounds->x1))
		|| !OD_CHECK(odBounds_check_valid_coord(bounds->y1))
		|| !OD_CHECK(odBounds_check_valid_coord(bounds->x2) && (bounds->x2 >= bounds->x1))
		|| !OD_CHECK(odBounds_check_valid_coord(bounds->y2) && (bounds->y2 >= bounds->y1))) {
		return false;
	}

	return true;
}
bool odBounds_is_collidable(const odBounds* bounds) {
	if (!OD_DEBUG_CHECK(odBounds_check_valid(bounds))) {
		return false;
	}

	if ((bounds->x2 <= bounds->x1)
		|| (bounds->y2 <= bounds->y1)) {
		return false;
	}

	return true;
}
bool odBounds_collides(const odBounds* a, const odBounds* b) {
	if (!OD_DEBUG_CHECK(odBounds_check_valid(a))
		|| !OD_DEBUG_CHECK(odBounds_check_valid(b))
		|| !OD_DEBUG_CHECK(odBounds_is_collidable(a))
		|| !OD_DEBUG_CHECK(odBounds_is_collidable(b))) {
		return false;
	}

	if ((a->x2 <= b->x1)
		|| (a->x1 >= b->x2)
		|| (a->y2 <= b->y1)
		|| (a->y1 >= b->y2)) {
		return false;
	}

	return true;
}
bool odBounds_equals(const odBounds* a, const odBounds* b) {
	if (!OD_DEBUG_CHECK(odBounds_check_valid(a))
		|| !OD_DEBUG_CHECK(odBounds_check_valid(b))) {
		return false;
	}

	if ((a->x1 != b->x1)
		|| (a->y1 != b->y1)
		|| (a->x2 != b->x2)
		|| (a->y2 != b->y2)) {
		return false;
	}

	return true;
}
float odBounds_get_width(const odBounds* bounds) {
	if (!OD_DEBUG_CHECK(odBounds_check_valid(bounds))) {
		return 0.0f;
	}

	return (bounds->x2 - bounds->x1);
}
float odBounds_get_height(const odBounds* bounds) {
	if (!OD_DEBUG_CHECK(odBounds_check_valid(bounds))) {
		return 0.0f;
	}

	return (bounds->y2 - bounds->y1);
}
void odBounds_floor(odBounds* bounds) {
	if (!OD_DEBUG_CHECK(odBounds_check_valid(bounds))) {
		return;
	}

	bounds->x1 = floorf(bounds->x1);
	bounds->y1 = floorf(bounds->y1);
	bounds->x2 = floorf(bounds->x2);
	bounds->y2 = floorf(bounds->y2);
}
