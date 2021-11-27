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
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_check_valid(const struct odBounds* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_collides(const struct odBounds* a, const struct odBounds* b);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_equals(const struct odBounds* a, const struct odBounds* b);
OD_API_C OD_CORE_MODULE void
odBounds_floor(struct odBounds* bounds);
