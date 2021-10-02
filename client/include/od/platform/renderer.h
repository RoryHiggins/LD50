#pragma once

#include <od/platform/module.h>

struct odType;

struct odRenderer;

OD_API_C OD_ENGINE_PLATFORM_MODULE const struct odType* odRenderer_get_type_constructor(void);
OD_API_C OD_ENGINE_PLATFORM_MODULE void odRenderer_swap(struct odRenderer* renderer1, struct odRenderer* renderer2);
OD_API_C OD_ENGINE_PLATFORM_MODULE
const char* odRenderer_get_debug_string(const struct odRenderer* renderer);
OD_API_C OD_ENGINE_PLATFORM_MODULE bool odRenderer_init(odRenderer* renderer, void* render_context_native);
OD_API_C OD_ENGINE_PLATFORM_MODULE void odRenderer_destroy(odRenderer* renderer);
