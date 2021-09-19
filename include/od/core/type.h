#pragma once

#include <od/core/module.h>

struct odType {
	int32_t size;
	void (*default_construct_fn)(void* ptr, int32_t count);
	void (*move_assign_fn)(void* ptr, void* src_ptr, int32_t count);
	void (*destruct_fn)(void* ptr, int32_t count);
};

OD_API_C OD_ENGINE_CORE_MODULE const char* odType_get_debug_string(const struct odType* type);
OD_API_C OD_ENGINE_CORE_MODULE void* odType_index(const struct odType* type, void* array, int32_t i);
OD_API_C OD_ENGINE_CORE_MODULE const void* odType_index_const(const struct odType* type, const void* array, int32_t i);
OD_API_C OD_ENGINE_CORE_MODULE const struct odType* odType_get_char(void);
