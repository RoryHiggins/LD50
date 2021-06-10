#pragma once

#include <od/platform/api.h>

struct odColor;
struct odImage;

OD_API_PLATFORM_C const struct odType* odImage_get_type_constructor(void);
OD_API_PLATFORM_C bool odImage_copy(struct odImage* image, const struct odImage* src_image);
OD_API_PLATFORM_C void odImage_swap(struct odImage* image1, struct odImage* image2);
OD_API_PLATFORM_C const char* odImage_get_debug_string(const struct odImage* image);
OD_API_PLATFORM_C bool odImage_allocate(struct odImage* image, uint32_t width, uint32_t height);
OD_API_PLATFORM_C void odImage_release(struct odImage* image);
OD_API_PLATFORM_C bool odImage_read_png(struct odImage* image, const void* src_png, uint32_t src_png_size);
OD_API_PLATFORM_C void odImage_get_size(const struct odImage* image, uint32_t* out_opt_width, uint32_t* out_opt_height);
OD_API_PLATFORM_C struct odColor* odImage_get(struct odImage* image);
OD_API_PLATFORM_C const struct odColor* odImage_get_const(const struct odImage* image);
