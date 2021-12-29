#include <od/core/math.h>

#include <cmath>
#include <cfloat>

#define OD_FLOAT_PRECISE_INT_BITS 24
#define OD_FLOAT_PRECISE_INT_MAX (1 << OD_FLOAT_PRECISE_INT_BITS)
#define OD_FLOAT_PRECISE_INT_MIN (-OD_FLOAT_PRECISE_INT_MAX)

bool odFloat_is_precise_int(float x) {
	if (!std::isfinite(x)
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
