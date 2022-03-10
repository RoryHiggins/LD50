#include <od/core/math.h>

#include <cmath>
#include <cfloat>

#if defined(__cplusplus) && (__cplusplus >= 202002)
#include <bit>
#endif

bool odFloat_is_precise_int24(float x) {
	if (!std::isfinite(x)
		|| (floorf(x) != x)
		|| (x < OD_FLOAT_PRECISE_INT_MIN)
		|| (x > OD_FLOAT_PRECISE_INT_MAX)) {
		return false;
	}

	return true;
}
bool odFloat_is_precise_uint8(float x) {
	if (!std::isfinite(x)
		|| (floorf(x) != x)
		|| (x < 0)
		|| (x > 0xFF)) {
		return false;
	}

	return true;
}
bool odFloat_epsilon_get_equals(float x, float y) {
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

int32_t odUint32_popcount(uint32_t x) {
#if defined(__cplusplus) && (__cplusplus >= 202002)
	return static_cast<int32_t>(std::popcount(x));
#elif defined(__clang__) || defined(__GNUC__)
	return static_cast<int32_t>(__builtin_popcount(x));
#else
	int32_t count = 0;
	for (int32_t i = 0; i < 32; i++) {
		if ((x & (1 << i)) > 0) {
			count++;
		}
	}
	return count;
#endif
}
