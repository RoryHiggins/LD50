#pragma once

#include <od/core/module.h>

struct odBounds {
	int32_t x1;
	int32_t y1;
	int32_t x2;
	int32_t y2;
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_check_valid(const struct odBounds* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odBounds_get_debug_string(const struct odBounds* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_fits_float(const struct odBounds* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_is_collidable(const struct odBounds* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_collides(const struct odBounds* a, const struct odBounds* b);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_contains(const struct odBounds* outer, const struct odBounds* inner);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBounds_equals(const struct odBounds* a, const struct odBounds* b);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD int32_t
odBounds_get_width(const struct odBounds* bounds);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD int32_t
odBounds_get_height(const struct odBounds* bounds);
