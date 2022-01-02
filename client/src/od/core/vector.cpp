#include <od/core/vector.h>

#include <cmath>

#include <od/core/debug.h>
#include <od/core/math.h>

bool odVector_check_valid(const odVector* vector) {
	if (!OD_CHECK(vector != nullptr)
		|| !OD_CHECK(std::isfinite(vector->vector[0]))
		|| !OD_CHECK(std::isfinite(vector->vector[1]))
		|| !OD_CHECK(std::isfinite(vector->vector[2]))) {
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
		static_cast<double>(vector->vector[0]),
		static_cast<double>(vector->vector[1]),
		static_cast<double>(vector->vector[2]),
		static_cast<double>(vector->vector[3]));
}
bool odVector_equals(const odVector* vector1, const odVector* vector2) {
	if (!OD_DEBUG_CHECK(odVector_check_valid(vector1))
		|| !OD_DEBUG_CHECK(odVector_check_valid(vector2))) {
		return false;
	}

	for (int32_t i = 0; i < OD_VECTOR4_ELEM_COUNT; i++) {
		if (vector1->vector[i] != vector2->vector[i]) {
			return false;
		}
	}

	return true;
}
bool odVector_epsilon_equals(const odVector* vector1, const odVector* vector2) {
	if (!OD_DEBUG_CHECK(odVector_check_valid(vector1))
		|| !OD_DEBUG_CHECK(odVector_check_valid(vector2))) {
		return false;
	}

	for (int32_t i = 0; i < OD_VECTOR4_ELEM_COUNT; i++) {
		if (!odFloat_epsilon_equals(vector1->vector[i], vector2->vector[i])) {
			return false;
		}
	}

	return true;
}
