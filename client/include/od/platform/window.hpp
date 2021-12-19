#pragma once

#include <od/platform/window.h>

struct odWindow {
	struct odWindowSettings settings;
	void* window_native;
	void* render_context_native;
	bool is_sdl_init;
	bool is_open;
	int32_t next_frame_ms;

	OD_PLATFORM_MODULE odWindow();
	OD_PLATFORM_MODULE odWindow(odWindow&& other);
	OD_PLATFORM_MODULE odWindow& operator=(odWindow&& other);
	OD_PLATFORM_MODULE ~odWindow();

	odWindow(odWindow const& other) = delete;
	odWindow& operator=(const odWindow& other) = delete;
};
struct odWindowScope {
	void* window_native;
	void* old_render_context_native;

	OD_PLATFORM_MODULE odWindowScope();
	OD_PLATFORM_MODULE ~odWindowScope();

	odWindowScope(const odWindowScope&) = delete;
	odWindowScope(odWindowScope&&) = delete;
	odWindowScope& operator=(const odWindowScope&) = delete;
	odWindowScope& operator=(odWindowScope&&) = delete;
};
