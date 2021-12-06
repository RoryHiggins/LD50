#include <od/core/bounds.h>

#include <cmath>

#include <od/core/debug.h>

const char* odBounds_get_debug_string(const odBounds* bounds) {
	if (bounds == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"x\": %g, \"y\": %g, \"width\": %g, \"height\": %g}",
		static_cast<double>(bounds->x),
		static_cast<double>(bounds->y),
		static_cast<double>(bounds->width),
		static_cast<double>(bounds->height));
}
bool odBounds_check_valid(const struct odBounds* bounds) {
	auto is_valid_signed = [](float x) {
		const float max = (1 << 24);
		const float min = -max;
		return std::isfinite(x) && (x >= min) && (x <= max);
	};
	auto is_valid_unsigned = [](float x) {
		const float max = (1 << 24);
		return std::isfinite(x) && (x >= 0) && (x <= max);
	};

	if (!OD_CHECK(bounds != nullptr)
		|| !OD_CHECK(is_valid_signed(bounds->x))
		|| !OD_CHECK(is_valid_signed(bounds->y))
		|| !OD_CHECK(is_valid_unsigned(bounds->width))
		|| !OD_CHECK(is_valid_unsigned(bounds->height))) {
		return false;
	}

	return true;
}
bool odBounds_collides(const odBounds* a, const odBounds* b) {
	if (!OD_DEBUG_CHECK(odBounds_check_valid(a))
		|| !OD_DEBUG_CHECK(odBounds_check_valid(b))) {
		return false;
	}

	if (((a->x + a->width) <= b->x)
		|| (a->x >= (b->x + b->width))
		|| ((a->y + a->height) <= b->y)
		|| (a->y >= (b->y + b->height))
		|| (a->width == 0)
		|| (b->height == 0)
		|| (b->height == 0)) {
		return false;
	}

	return true;
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
