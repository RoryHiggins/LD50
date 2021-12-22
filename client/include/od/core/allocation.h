#pragma once

#include <od/core/module.h>

struct odAllocation;

OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odAllocation_check_valid(const struct odAllocation* allocation);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odAllocation_get_debug_string(const struct odAllocation* allocation);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odAllocation_init(struct odAllocation* allocation, int32_t size);
OD_API_C OD_CORE_MODULE void
odAllocation_destroy(struct odAllocation* allocation);
OD_API_C OD_CORE_MODULE void
odAllocation_swap(struct odAllocation* allocation1, struct odAllocation* allocation2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD void*
odAllocation_get(struct odAllocation* allocation);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const void*
odAllocation_get_const(const struct odAllocation* allocation);
