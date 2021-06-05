#pragma once

#include <od/core/api.h>

struct odType;
struct odBox;

OD_API_C const struct odType* odBox_get_type_constructor(void);
OD_API_C void odBox_swap(struct odBox* ptr1, struct odBox* ptr2);
OD_API_C const char* odBox_get_debug_string(const struct odBox* ptr);
OD_API_C const struct odType* odBox_get_type(const struct odBox* ptr);
OD_API_C void odBox_set_type(struct odBox* ptr, const struct odType* type);
OD_API_C bool odBox_allocate(struct odBox* ptr);
OD_API_C void odBox_release(struct odBox* ptr);
OD_API_C void* odBox_get(struct odBox* ptr);
OD_API_C const void* odBox_get_const(const struct odBox* ptr);
