#pragma once

#include <od/platform/module.h>

#include <od/core/bounds.h>
#include <od/core/matrix.h>

struct odWindow;

struct odTexture;

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const struct odType*
odTexture_get_type_constructor(void);
OD_API_C OD_PLATFORM_MODULE void
odTexture_swap(struct odTexture* texture1, struct odTexture* texture2);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odTexture_check_valid(const struct odTexture* texture);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const char*
odTexture_get_debug_string(const struct odTexture* texture);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odTexture_init(struct odTexture* texture, struct odWindow* window,
			   const struct odColor* opt_pixels, int32_t width, int32_t height);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odTexture_init_blank(struct odTexture* texture, struct odWindow* window);
OD_API_C OD_PLATFORM_MODULE void
odTexture_destroy(struct odTexture* texture);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odTexture_get_size(const struct odTexture* texture, int32_t* out_opt_width, int32_t* out_opt_height);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const struct odType*
odRenderTexture_get_type_constructor(void);
OD_API_C OD_PLATFORM_MODULE void
odRenderTexture_swap(struct odRenderTexture* render_texture1, struct odRenderTexture* render_texture2);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odRenderTexture_check_valid(const struct odRenderTexture* render_texture);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const char*
odRenderTexture_get_debug_string(const struct odRenderTexture* render_texture);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odRenderTexture_init(struct odRenderTexture* render_texture, struct odWindow* window,
					 int32_t width, int32_t height);
OD_API_C OD_PLATFORM_MODULE void
odRenderTexture_destroy(struct odRenderTexture* render_texture);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD struct odTexture*
odRenderTexture_get_texture(struct odRenderTexture* render_texture);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const struct odTexture*
odRenderTexture_get_texture_const(const struct odRenderTexture* render_texture);
