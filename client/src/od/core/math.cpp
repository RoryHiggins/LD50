#include <od/core/math.h>

#include <cmath>
#include <cfloat>

bool odFloat_is_precise_int(float x) {
	if (!std::isfinite(x)
		|| (floorf(x) != x)
		|| (x < OD_FLOAT_PRECISE_INT_MIN)
		|| (x > OD_FLOAT_PRECISE_INT_MAX)) {
		return false;
	}

	return true;
}
bool odFloat_epsilon_equals(float x, float y) {
	if (!std::isfinite(x)
		|| !std::isfinite(y)) {
		return false;
	}
	return fabsf(y - x) <= FLT_EPSILON;
}

bool odInt32_fits_float(int32_t x) {
	if ((x < OD_FLOAT_PRECISE_INT_MIN)
		|| (x > OD_FLOAT_PRECISE_INT_MAX)) {
		return false;
	}

	return true;
}
