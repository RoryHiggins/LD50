#pragma once

#include <od/core/module.h>

struct odType;
struct odAllocation;

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
const struct odType* odAllocation_get_type_constructor(void);

OD_API_C OD_ENGINE_CORE_MODULE
void odAllocation_swap(struct odAllocation* allocation1, struct odAllocation* allocation2);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
const char* odAllocation_get_debug_string(const struct odAllocation* allocation);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
bool odAllocation_get_valid(const struct odAllocation* allocation);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
bool odAllocation_allocate(struct odAllocation* allocation, int32_t size);

OD_API_C OD_ENGINE_CORE_MODULE
void odAllocation_release(struct odAllocation* allocation);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
void* odAllocation_get(struct odAllocation* allocation);

OD_API_C OD_ENGINE_CORE_MODULE OD_API_NODISCARD
const void* odAllocation_get_const(const struct odAllocation* allocation);

