#pragma once

#include <od/core/module.h>

#include <od/core/vector.h>
#include <od/core/color.h>

struct odVertex {
	odVector4 pos;
	odColor col;
	float u;
	float v;
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odVertex_get_debug_string(const struct odVertex* vertex);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVertex_check_valid(const struct odVertex* vertex);
