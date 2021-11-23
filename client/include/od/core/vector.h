#pragma once

#include <od/core/module.h>

struct odVector {
	float vector[4];  // xyzw
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odVector_get_debug_string(const struct odVector* vector);
