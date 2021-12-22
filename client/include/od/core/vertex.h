#pragma once

#include <od/core/module.h>

#include <od/core/vector.h>
#include <od/core/color.h>

struct odMatrix4;

struct odVertex {
	odVector4 pos;
	odColor color;
	float u;
	float v;
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odVertex_get_debug_string(const struct odVertex* vertex);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVertex_check_valid(const struct odVertex* vertex);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVertex_check_valid_batch(const struct odVertex* vertices, int32_t vertices_count);
OD_API_C OD_CORE_MODULE void
odVertex_transform(struct odVertex* vertex, const struct odMatrix4* matrix);
OD_API_C OD_CORE_MODULE void
odVertex_transform_batch(struct odVertex* vertices, int32_t vertices_count, const struct odMatrix4* matrix);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odVertex_less(const struct odVertex* vertex, const struct odVertex* vertex2);
