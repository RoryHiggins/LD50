#pragma once

#include <od/core/module.h>

#define OD_DOUBLE_PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062

#define OD_FLOAT_PI float(OD_DOUBLE_PI)
#define OD_FLOAT_PRECISE_INT_BITS 24
#define OD_FLOAT_PRECISE_INT_MAX (1 << OD_FLOAT_PRECISE_INT_BITS)
#define OD_FLOAT_PRECISE_INT_MIN (-OD_FLOAT_PRECISE_INT_MAX)

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odFloat_is_normalized(float x);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odFloat_is_precise_int24(float x);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odFloat_is_precise_uint8(float x);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odFloat_epsilon_get_equals(float x, float y);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odInt32_fits_float(int32_t x);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD int32_t
odUint32_popcount(uint32_t x);
