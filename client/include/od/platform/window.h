#pragma once

#include <od/platform/module.h>

struct odType;

struct odWindow;

struct odWindowSettings {
	const char* caption;
	int32_t width;
	int32_t height;
	int32_t fps_limit;
	bool is_fps_limit_enabled;
	bool is_vsync_enabled;
	bool is_visible;
};

OD_API_C OD_ENGINE_PLATFORM_MODULE OD_API_NODISCARD
struct odWindowSettings odWindowSettings_get_defaults(void);

OD_API_C OD_ENGINE_PLATFORM_MODULE OD_API_NODISCARD
struct odWindowSettings odWindowSettings_get_headless_defaults(void);

OD_API_C OD_ENGINE_PLATFORM_MODULE OD_API_NODISCARD
const struct odType* odWindow_get_type_constructor(void);

OD_API_C OD_ENGINE_PLATFORM_MODULE
void odWindow_swap(struct odWindow* window1, struct odWindow* window2);

OD_API_C OD_ENGINE_PLATFORM_MODULE OD_API_NODISCARD
const char* odWindow_get_debug_string(const struct odWindow* window);

OD_API_C OD_ENGINE_PLATFORM_MODULE OD_API_NODISCARD
bool odWindow_init(struct odWindow* window, struct odWindowSettings settings);

OD_API_C OD_ENGINE_PLATFORM_MODULE
void odWindow_destroy(struct odWindow* window);

OD_API_C OD_ENGINE_PLATFORM_MODULE OD_API_NODISCARD
bool odWindow_step(struct odWindow* window);

OD_API_C OD_ENGINE_PLATFORM_MODULE OD_API_NODISCARD
bool odWindow_set_visible(struct odWindow* window, bool is_visible);

OD_API_C OD_ENGINE_PLATFORM_MODULE OD_API_NODISCARD
bool odWindow_set_size(struct odWindow* window, int32_t width, int32_t height);

OD_API_C OD_ENGINE_PLATFORM_MODULE OD_API_NODISCARD
bool odWindow_get_open(const struct odWindow* window);

OD_API_C OD_ENGINE_PLATFORM_MODULE OD_API_NODISCARD
const struct odWindowSettings* odWindow_get_settings(const struct odWindow* window);

