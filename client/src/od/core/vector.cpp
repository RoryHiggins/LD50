#include <od/core/vector.h>

#include <cmath>

#include <od/core/debug.h>
#include <od/core/math.h>

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
bool odVector4_check_valid_3d(const odVector4* vector3) {
	if (!OD_CHECK(odVector4_check_valid(vector3))
		|| !OD_CHECK(vector3->vector[3] == 1)
		|| !OD_CHECK(odFloat_is_precise_int(vector3->vector[2]))) {
		return false;
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
void odVector4_init_3d(odVector4* vector, float x, float y, float z) {
	if (!OD_DEBUG_CHECK(vector != nullptr)) {
		return;
	}
	OD_DISCARD(
		OD_DEBUG_CHECK(std::isfinite(x))
		&& OD_DEBUG_CHECK(std::isfinite(y))
		&& OD_DEBUG_CHECK(std::isfinite(z)));

	vector->vector[0] = x;
	vector->vector[1] = y;
	vector->vector[2] = z;
	vector->vector[3] = 1;  // w coordinate should always be 1
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
bool odVector4_epsilon_equals(const odVector4* vector1, const odVector4* vector2) {
	if (!OD_DEBUG_CHECK(vector1 != nullptr)
		|| !OD_DEBUG_CHECK(vector2 != nullptr)) {
		return false;
	}

	for (int32_t i = 0; i < OD_VECTOR4_ELEM_COUNT; i++) {
		if (!odFloat_epsilon_equals(vector1->vector[i], vector2->vector[i])) {
			return false;
		}
	}

	return true;
}
