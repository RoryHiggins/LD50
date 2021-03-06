#include <od/core/bounds.h>

#include <cmath>

#include <od/core/debug.h>
#include <od/core/math.h>

bool odBounds_check_valid(const odBounds* bounds) {
	if (!OD_CHECK(bounds != nullptr)
		|| !OD_CHECK(bounds->x2 >= bounds->x1)
		|| !OD_CHECK(bounds->y2 >= bounds->y1)
		|| !OD_CHECK(odFloat_is_precise_int24(bounds->x1))
		|| !OD_CHECK(odFloat_is_precise_int24(bounds->y1))
		|| !OD_CHECK(odFloat_is_precise_int24(bounds->x2))
		|| !OD_CHECK(odFloat_is_precise_int24(bounds->y2))) {
		return false;
	}

	return true;
}
const char* odBounds_get_debug_string(const odBounds* bounds) {
	if (bounds == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"x1\": %g, \"y1\": %g, \"x2\": %g, \"y2\": %g}",
		static_cast<double>(bounds->x1),
		static_cast<double>(bounds->y1),
		static_cast<double>(bounds->x2),
		static_cast<double>(bounds->y2));
}
bool odBounds_has_area(const odBounds* bounds) {
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
		|| !OD_DEBUG_CHECK(odBounds_has_area(a))
		|| !OD_DEBUG_CHECK(odBounds_has_area(b))) {
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
bool odBounds_contains(const odBounds* outer, const odBounds* inner) {
	if (!OD_DEBUG_CHECK(odBounds_check_valid(outer))
		|| !OD_DEBUG_CHECK(odBounds_check_valid(inner))
		|| !OD_DEBUG_CHECK(odBounds_has_area(outer))
		|| !OD_DEBUG_CHECK(odBounds_has_area(inner))) {
		return false;
	}

	if ((inner->x1 < outer->x1)
		|| (inner->x2 > outer->x2)
		|| (inner->y1 < outer->y1)
		|| (inner->y2 > outer->y2)) {
		return false;
	}

	return true;
}
bool odBounds_get_equals(const odBounds* a, const odBounds* b) {
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
