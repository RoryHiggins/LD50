#pragma once

#include <od/core/module.h>

struct odBounds {
	float x1;
	float y1;
	float x2;
	float y2;
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_check_valid(const struct odBounds* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odBounds_get_debug_string(const struct odBounds* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_has_area(const struct odBounds* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_collides(const struct odBounds* a, const struct odBounds* b);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_contains(const struct odBounds* outer, const struct odBounds* inner);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_get_equals(const struct odBounds* a, const struct odBounds* b);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD float
odBounds_get_width(const struct odBounds* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD float
odBounds_get_height(const struct odBounds* bounds);
