#pragma once

#include <od/core/api.h>

struct odType;
struct odAllocation;

OD_API_CORE_C const struct odType* odAllocation_get_type_constructor(void);
OD_API_CORE_C void odAllocation_swap(struct odAllocation* allocation1, struct odAllocation* allocation2);
OD_API_CORE_C const char* odAllocation_get_debug_string(const struct odAllocation* allocation);
OD_API_CORE_C bool odAllocation_allocate(struct odAllocation* allocation, uint32_t size);
OD_API_CORE_C void odAllocation_release(struct odAllocation* allocation);
OD_API_CORE_C void* odAllocation_get(struct odAllocation* allocation);
OD_API_CORE_C const void* odAllocation_get_const(const struct odAllocation* allocation);
