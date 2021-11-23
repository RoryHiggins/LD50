#pragma once

#include <od/core/module.h>

struct odBounds {
	float x;
	float y;
	float width;
	float height;
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odBounds_get_debug_string(const struct odBounds* bounds);
