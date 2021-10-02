#pragma once

#include <od/platform/window.h>

#include <od/platform/renderer.hpp>

struct odGLRenderer;
struct odWindow {
	odRenderer renderer;
	void* window_native;
	void* render_context_native;
	bool is_sdl_init;
	bool is_open;
	int32_t next_frame_ms;
	struct odWindowSettings settings;

	OD_ENGINE_PLATFORM_MODULE odWindow();
	OD_ENGINE_PLATFORM_MODULE odWindow(odWindow&& other);
	OD_ENGINE_PLATFORM_MODULE odWindow& operator=(odWindow&& other);
	OD_ENGINE_PLATFORM_MODULE ~odWindow();

	odWindow(odWindow const& other) = delete;
	odWindow& operator=(const odWindow& other) = delete;
};
