#pragma once

#include <od/core/api.h>

struct odType {
	uint32_t size;
	void (*default_construct_fn)(void* ptr, uint32_t count);
	void (*move_assign_fn)(void* ptr, void* src_ptr, uint32_t count);
	void (*destruct_fn)(void* ptr, uint32_t count);
};

OD_API_ENGINE_C const char* odType_get_debug_string(const struct odType* type);
OD_API_ENGINE_C void* odType_index(const struct odType* type, void* array, uint32_t i);
OD_API_ENGINE_C const void* odType_index_const(const struct odType* type, const void* array, uint32_t i);
OD_API_ENGINE_C const struct odType* odType_get_char(void);
