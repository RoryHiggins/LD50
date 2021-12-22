#include <od/core/vector.h>

#include <cmath>

#include <od/core/debug.h>

bool odVector4_check_valid(const odVector4* vector) {
	if (!OD_CHECK(vector != nullptr)) {
		return false;
	}

	for (int32_t i = 0; i < OD_VECTOR4_ELEM_COUNT; i++) {
		if (!OD_CHECK(std::isfinite(vector->vector[i]))) {
			return false;
		}
	}

	return true;
}
const char* odVector4_get_debug_string(const odVector4* vector) {
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
bool odVector4_equals(const odVector4* vector1, const odVector4* vector2) {
	if (!OD_DEBUG_CHECK(vector1 != nullptr)
		|| !OD_DEBUG_CHECK(vector2 != nullptr)) {
		return false;
	}

	for (int32_t i = 0; i < OD_VECTOR4_ELEM_COUNT; i++) {
		if (vector1->vector[i] != vector2->vector[i]) {
			return false;
		}
	}

	return true;
}
