#pragma once

#include <od/core/module.h>

struct odBounds2 {
	float x1;
	float y1;
	float x2;
	float y2;
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds2_check_valid(const struct odBounds2* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odBounds2_get_debug_string(const struct odBounds2* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds2_is_collidable(const struct odBounds2* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds2_collides(const struct odBounds2* a, const struct odBounds2* b);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds2_equals(const struct odBounds2* a, const struct odBounds2* b);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD float
odBounds2_get_width(const struct odBounds2* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD float
odBounds2_get_height(const struct odBounds2* bounds);
OD_API_C OD_CORE_MODULE void
odBounds2_floor(struct odBounds2* bounds);
