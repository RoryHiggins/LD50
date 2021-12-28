#pragma once

#include <od/platform/module.h>

#include <od/core/bounds.h>
#include <od/core/matrix.h>

struct odType;
struct odWindow;
struct odTexture;
struct odRenderTexture;

struct odRenderer;

struct odRenderState {
	struct odMatrix4 view;
	struct odMatrix4 projection;
	struct odBounds2 viewport;
	const struct odTexture* src_texture;
	struct odRenderTexture* opt_render_texture;
};

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odRenderState_check_valid(const struct odRenderState* state);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const struct odType*
odRenderer_get_type_constructor(void);
OD_API_C OD_PLATFORM_MODULE void
odRenderer_swap(struct odRenderer* renderer1, struct odRenderer* renderer2);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odRenderer_check_valid(const struct odRenderer* renderer);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const char*
odRenderer_get_debug_string(const struct odRenderer* renderer);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odRenderer_init(struct odRenderer* renderer, struct odWindow* window);
OD_API_C OD_PLATFORM_MODULE void
odRenderer_destroy(struct odRenderer* renderer);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odRenderer_flush(struct odRenderer* renderer);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odRenderer_clear(struct odRenderer* renderer, struct odRenderState* state, const struct odColor* color);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odRenderer_draw_vertices(struct odRenderer* renderer, struct odRenderState* state,
						 const struct odVertex* vertices, int32_t vertices_count);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odRenderer_draw_texture(struct odRenderer* renderer, struct odRenderState* state,
						const struct odBounds2* opt_src_bounds, const struct odMatrix4* opt_transform);
