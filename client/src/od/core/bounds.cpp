#include <od/core/bounds.h>

#include <cmath>

#include <od/core/debug.h>
#include <od/core/math.h>

bool odBounds2f_check_valid(const odBounds2f* bounds) {
	if (!OD_CHECK(bounds != nullptr)
		|| !OD_CHECK(odFloat_is_precise_int(bounds->x1))
		|| !OD_CHECK(odFloat_is_precise_int(bounds->y1))
		|| !OD_CHECK(odFloat_is_precise_int(bounds->x2) && (bounds->x2 >= bounds->x1))
		|| !OD_CHECK(odFloat_is_precise_int(bounds->y2) && (bounds->y2 >= bounds->y1))) {
		return false;
	}

	return true;
}
const char* odBounds2f_get_debug_string(const odBounds2f* bounds) {
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
bool odBounds2f_is_collidable(const odBounds2f* bounds) {
	if (!OD_DEBUG_CHECK(odBounds2f_check_valid(bounds))) {
		return false;
	}

	if ((bounds->x2 <= bounds->x1)
		|| (bounds->y2 <= bounds->y1)) {
		return false;
	}

	return true;
}
bool odBounds2f_collides(const odBounds2f* a, const odBounds2f* b) {
	if (!OD_DEBUG_CHECK(odBounds2f_check_valid(a))
		|| !OD_DEBUG_CHECK(odBounds2f_check_valid(b))
		|| !OD_DEBUG_CHECK(odBounds2f_is_collidable(a))
		|| !OD_DEBUG_CHECK(odBounds2f_is_collidable(b))) {
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
bool odBounds2f_equals(const odBounds2f* a, const odBounds2f* b) {
	if (!OD_DEBUG_CHECK(odBounds2f_check_valid(a))
		|| !OD_DEBUG_CHECK(odBounds2f_check_valid(b))) {
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
float odBounds2f_get_width(const odBounds2f* bounds) {
	if (!OD_DEBUG_CHECK(odBounds2f_check_valid(bounds))) {
		return 0.0f;
	}

	return (bounds->x2 - bounds->x1);
}
float odBounds2f_get_height(const odBounds2f* bounds) {
	if (!OD_DEBUG_CHECK(odBounds2f_check_valid(bounds))) {
		return 0.0f;
	}

	return (bounds->y2 - bounds->y1);
}
void odBounds2f_floor(odBounds2f* bounds) {
	if (!OD_DEBUG_CHECK(odBounds2f_check_valid(bounds))) {
		return;
	}

	bounds->x1 = floorf(bounds->x1);
	bounds->y1 = floorf(bounds->y1);
	bounds->x2 = floorf(bounds->x2);
	bounds->y2 = floorf(bounds->y2);
}
