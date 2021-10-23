#pragma once

#include <od/platform/module.h>

struct odType;
struct odColor;

struct odTexture;

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
const struct odType* odTexture_get_type_constructor(void);

OD_API_C OD_PLATFORM_MODULE
void odTexture_swap(struct odTexture* texture1, struct odTexture* texture2);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
bool odTexture_get_valid(const struct odTexture* texture);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
const char* odTexture_get_debug_string(const struct odTexture* texture);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
bool odTexture_init(struct odTexture* texture, void* render_context_native, const struct odColor* opt_pixels, int32_t width, int32_t height);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD
bool odTexture_init_blank(struct odTexture* texture, void* render_context_native);

OD_API_C OD_PLATFORM_MODULE
void odTexture_destroy(struct odTexture* texture);

OD_API_C OD_PLATFORM_MODULE
void odTexture_get_size(const struct odTexture* texture, int32_t* out_opt_width, int32_t* out_opt_height);
