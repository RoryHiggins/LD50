#include <od/core/vector.h>

#include <cmath>

#include <od/core/debug.h>
#include <od/core/math.h>

bool odVector_check_valid(const odVector* vector) {
	if (!OD_CHECK(vector != nullptr)
		|| !OD_CHECK(std::isfinite(vector->x))
		|| !OD_CHECK(std::isfinite(vector->y))
		|| !OD_CHECK(std::isfinite(vector->z))
		|| !OD_CHECK(std::isfinite(vector->w))) {
		return false;
	}

	return true;
}
bool odVector_check_valid_3d(const odVector* vector) {
	if (!OD_CHECK(odVector_check_valid(vector))
		|| !OD_CHECK((vector->w == 0.0f) || (vector->w == 1.0f))) {
		return false;
	}

	return true;
}
const char* odVector_get_debug_string(const odVector* vector) {
	if (vector == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"\"[%g,%g,%g,%g]\"",
		static_cast<double>(vector->x),
		static_cast<double>(vector->y),
		static_cast<double>(vector->z),
		static_cast<double>(vector->w));
}
bool odVector_equals(const odVector* vector1, const odVector* vector2) {
	if (!OD_DEBUG_CHECK(odVector_check_valid(vector1))
		|| !OD_DEBUG_CHECK(odVector_check_valid(vector2))) {
		return false;
	}

	if ((vector1->x != vector2->x)
		|| (vector1->y != vector2->y)
		|| (vector1->z != vector2->z)
		|| (vector1->w != vector2->w)) {
		return false;
	}

	return true;
}
bool odVector_epsilon_equals(const odVector* vector1, const odVector* vector2) {
	if (!OD_DEBUG_CHECK(odVector_check_valid(vector1))
		|| !OD_DEBUG_CHECK(odVector_check_valid(vector2))) {
		return false;
	}

	if (!odFloat_epsilon_equals(vector1->x, vector2->x)
		|| !odFloat_epsilon_equals(vector1->y, vector2->y)
		|| !odFloat_epsilon_equals(vector1->z, vector2->z)
		|| !odFloat_epsilon_equals(vector1->w, vector2->w)) {
		return false;
	}

	return true;
}
