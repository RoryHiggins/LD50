#pragma once

#include <od/core/module.h>

#define OD_DOUBLE_PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062
#define OD_FLOAT_PI float(OD_DOUBLE_PI)

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odFloat_is_precise_int(float x);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odFloat_epsilon_equals(float x, float y);
