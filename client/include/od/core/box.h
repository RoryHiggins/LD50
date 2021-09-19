#pragma once

#include <od/core/module.h>

struct odType;
struct odBox;

OD_API_C OD_ENGINE_CORE_MODULE const struct odType* odBox_get_type_constructor(void);
OD_API_C OD_ENGINE_CORE_MODULE void odBox_swap(struct odBox* box1, struct odBox* box2);
OD_API_C OD_ENGINE_CORE_MODULE const char* odBox_get_debug_string(const struct odBox* box);
OD_API_C OD_ENGINE_CORE_MODULE const struct odType* odBox_get_type(const struct odBox* box);
OD_API_C OD_ENGINE_CORE_MODULE void odBox_set_type(struct odBox* box, const struct odType* type);
OD_API_C OD_ENGINE_CORE_MODULE bool odBox_allocate(struct odBox* box);
OD_API_C OD_ENGINE_CORE_MODULE void odBox_release(struct odBox* box);
OD_API_C OD_ENGINE_CORE_MODULE void* odBox_get(struct odBox* box);
OD_API_C OD_ENGINE_CORE_MODULE const void* odBox_get_const(const struct odBox* box);
