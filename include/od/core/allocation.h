#pragma once

#include <od/core/module.h>

struct odType;
struct odAllocation;

OD_API_C OD_ENGINE_CORE_MODULE const struct odType* odAllocation_get_type_constructor(void);
OD_API_C OD_ENGINE_CORE_MODULE void
odAllocation_swap(struct odAllocation* allocation1, struct odAllocation* allocation2);
OD_API_C OD_ENGINE_CORE_MODULE const char* odAllocation_get_debug_string(const struct odAllocation* allocation);
OD_API_C OD_ENGINE_CORE_MODULE bool odAllocation_allocate(struct odAllocation* allocation, uint32_t size);
OD_API_C OD_ENGINE_CORE_MODULE void odAllocation_release(struct odAllocation* allocation);
OD_API_C OD_ENGINE_CORE_MODULE void* odAllocation_get(struct odAllocation* allocation);
OD_API_C OD_ENGINE_CORE_MODULE const void* odAllocation_get_const(const struct odAllocation* allocation);
