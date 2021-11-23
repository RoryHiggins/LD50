#pragma once

#include <od/core/module.h>

struct odColor {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odColor_get_debug_string(const struct odColor* color);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odColor*
odColor_get_white(void);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odColor*
odColor_get_black(void);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odColor*
odColor_get_red(void);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odColor*
odColor_get_green(void);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odColor*
odColor_get_blue(void);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odColor*
odColor_get_transparent(void);
