#pragma once

#include <od/platform/module.h>

struct odType;
struct odColor;
struct odVertex;

struct odRenderer;

struct odViewport {
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
};

OD_API_C OD_ENGINE_PLATFORM_MODULE const struct odType* odRenderer_get_type_constructor(void);
OD_API_C OD_ENGINE_PLATFORM_MODULE void odRenderer_swap(struct odRenderer* renderer1, struct odRenderer* renderer2);
OD_API_C OD_ENGINE_PLATFORM_MODULE
const char* odRenderer_get_debug_string(const struct odRenderer* renderer);
OD_API_C OD_ENGINE_PLATFORM_MODULE bool odRenderer_init(struct odRenderer* renderer, void* render_context_native);
OD_API_C OD_ENGINE_PLATFORM_MODULE void odRenderer_destroy(struct odRenderer* renderer);
OD_API_C OD_ENGINE_PLATFORM_MODULE bool odRenderer_draw(struct odRenderer* renderer, const struct odVertex* vertices, int32_t vertices_count, struct odViewport viewport);
