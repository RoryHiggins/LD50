#pragma once

#include <od/platform/api.h>

struct odWindow;

struct odWindowSettings {
	const char* caption;
	uint32_t width;
	uint32_t height;
	uint32_t fps_limit;
	bool is_fps_limited;
	bool is_visible;
};

OD_API_PLATFORM_C struct odWindowSettings odWindowSettings_get_defaults(void);

OD_API_PLATFORM_C const struct odType* odWindow_get_type_constructor(void);
OD_API_PLATFORM_C void odWindow_swap(struct odWindow* window1, struct odWindow* window2);
OD_API_PLATFORM_C const char* odWindow_get_debug_string(const struct odWindow* window);
OD_API_PLATFORM_C bool odWindow_open(struct odWindow* window, const odWindowSettings* settings);
OD_API_PLATFORM_C void odWindow_close(struct odWindow* window);
OD_API_PLATFORM_C void odWindow_step(struct odWindow* window);
OD_API_PLATFORM_C bool odWindow_set_visible(struct odWindow* window, bool is_visible);
OD_API_PLATFORM_C bool odWindow_get_open(const struct odWindow* window);
OD_API_PLATFORM_C const struct odWindowSettings* odWindow_get_settings(const struct odWindow* window);
OD_API_PLATFORM_C void* odWindow_get_native_window(struct odWindow* window);
OD_API_PLATFORM_C void* odWindow_get_native_render_context(struct odWindow* window);
