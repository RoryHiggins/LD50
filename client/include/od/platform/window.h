#pragma once

#include <od/platform/module.h>

struct odType;
struct odWindow;
struct odWindowResource;
struct odWindowScope;

struct odWindowSettings {
	const char* caption;  // pointed-to string must outlive the window
	int32_t width;
	int32_t height;
	int32_t fps_limit;
	bool is_fps_limit_enabled;
	bool is_vsync_enabled;
	bool is_visible;
};
struct odWindowMouseState {
	int32_t x;
	int32_t y;
	bool is_left_down;
	bool is_middle_down;
	bool is_right_down;
};

OD_API_C OD_PLATFORM_MODULE const char*
odWindowSettings_get_debug_string(const struct odWindowSettings* settings);
OD_API_C OD_PLATFORM_MODULE const struct odWindowSettings*
odWindowSettings_get_defaults(void);
OD_API_C OD_PLATFORM_MODULE const struct odWindowSettings*
odWindowSettings_get_headless_defaults(void);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odWindowSettings_check_valid(const struct odWindowSettings* settings);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const struct odType*
odWindow_get_type_constructor(void);
OD_API_C OD_PLATFORM_MODULE void
odWindow_swap(struct odWindow* window1, struct odWindow* window2);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const char*
odWindow_get_debug_string(const struct odWindow* window);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odWindow_check_valid(const struct odWindow* window);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odWindow_init(struct odWindow* window, const struct odWindowSettings* opt_settings);
OD_API_C OD_PLATFORM_MODULE void
odWindow_destroy(struct odWindow* window);
OD_API_C OD_PLATFORM_MODULE void*
odWindow_prepare_render_context(struct odWindow* window);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odWindow_step(struct odWindow* window);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odWindow_set_visible(struct odWindow* window, bool is_visible);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odWindow_set_size(struct odWindow* window, int32_t width, int32_t height);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odWindow_set_settings(struct odWindow* window, const struct odWindowSettings* settings);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD const struct odWindowSettings*
odWindow_get_settings(const struct odWindow* window);
OD_API_C OD_PLATFORM_MODULE void
odWindow_get_mouse_state(const struct odWindow* window, struct odWindowMouseState* out_mouse_state);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odWindow_get_key_state(const struct odWindow* window, const char* key_name);


OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odWindowResource_init(struct odWindowResource* resource, struct odWindow* opt_window);
OD_API_C OD_PLATFORM_MODULE void
odWindowResource_destroy(struct odWindowResource* resource);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odWindowScope_bind(struct odWindowScope* scope, struct odWindow* window);
OD_API_C OD_PLATFORM_MODULE bool
odWindowScope_try_bind(struct odWindowScope* scope, struct odWindow* window);
