#pragma once

#include <od/core/module.h>

struct odBounds2f {
	float x1;
	float y1;
	float x2;
	float y2;
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds2f_check_valid(const struct odBounds2f* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odBounds2f_get_debug_string(const struct odBounds2f* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds2f_is_collidable(const struct odBounds2f* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds2f_collides(const struct odBounds2f* a, const struct odBounds2f* b);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds2f_equals(const struct odBounds2f* a, const struct odBounds2f* b);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD float
odBounds2f_get_width(const struct odBounds2f* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD float
odBounds2f_get_height(const struct odBounds2f* bounds);
OD_API_C OD_CORE_MODULE void
odBounds2f_floor(struct odBounds2f* bounds);
