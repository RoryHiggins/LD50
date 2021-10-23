#pragma once

#include <od/platform/module.h>

struct odType;
struct odColor;
struct odBounds;
struct odVertex;
struct odRenderTexture;

struct odRenderer;

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
const struct odType* odRenderer_get_type_constructor(void);

OD_API_C OD_PLATFORM_MODULE
void odRenderer_swap(struct odRenderer* renderer1, struct odRenderer* renderer2);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
bool odRenderer_get_valid(const struct odRenderer* renderer);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
const char* odRenderer_get_debug_string(const struct odRenderer* renderer);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
bool odRenderer_init(struct odRenderer* renderer, void* render_context_native);

OD_API_C OD_PLATFORM_MODULE
void odRenderer_destroy(struct odRenderer* renderer);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
bool odRenderer_flush(struct odRenderer* renderer);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
bool odRenderer_clear(struct odRenderer* renderer, odColor color, struct odRenderTexture* opt_render_texture);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
bool odRenderer_draw_vertices(struct odRenderer* renderer, struct odRenderState state, const struct odVertex* vertices, int32_t vertices_count);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
bool odRenderer_draw_texture(struct odRenderer* renderer, struct odRenderState state, const struct odBounds *src_texture_bounds);
