#pragma once

#include <od/core/module.h>

struct odType;

struct odBox;

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBox_check_valid(const struct odBox* box);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odBox_get_debug_string(const struct odBox* box);
OD_API_C OD_CORE_MODULE void
odBox_swap(struct odBox* box1, struct odBox* box2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odBox_init(struct odBox* box, const struct odType* type);
OD_API_C OD_CORE_MODULE void
odBox_init_null(struct odBox* box);
OD_API_C OD_CORE_MODULE void
odBox_destroy(struct odBox* box);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD void*
odBox_get(struct odBox* box);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const void*
odBox_get_const(const struct odBox* box);
OD_API_C OD_CORE_MODULE void*
odBox_ensure(struct odBox* box, const struct odType* type);
