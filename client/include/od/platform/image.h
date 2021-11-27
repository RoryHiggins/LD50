#pragma once

#include <od/platform/module.h>

struct odType;
struct odColor;
struct odImage;

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const struct odType*
odImage_get_type_constructor(void);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odImage_copy(struct odImage* image, const struct odImage* src_image);
OD_API_C OD_PLATFORM_MODULE void
odImage_swap(struct odImage* image1, struct odImage* image2);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odImage_check_valid(const struct odImage* image);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const char*
odImage_get_debug_string(const struct odImage* image);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odImage_allocate(struct odImage* image, int32_t width, int32_t height);
OD_API_C OD_PLATFORM_MODULE void
odImage_release(struct odImage* image);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odImage_read_png(struct odImage* image, const void* src_png, int32_t src_png_size);
OD_API_C OD_PLATFORM_MODULE void
odImage_get_size(const struct odImage* image, int32_t* out_opt_width, int32_t* out_opt_height);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD struct odColor*
odImage_get(struct odImage* image);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const struct odColor*
odImage_get_const(const struct odImage* image);

