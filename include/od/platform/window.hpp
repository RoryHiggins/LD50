#pragma once

#include <od/platform/window.h>

struct odWindow {
	void* window_native;
	void* renderer_native;
	bool is_open;
	uint32_t next_frame_ms;
	struct odWindowSettings settings;

	OD_ENGINE_PLATFORM_MODULE odWindow();
	OD_ENGINE_PLATFORM_MODULE odWindow(odWindow&& other);
	OD_ENGINE_PLATFORM_MODULE odWindow& operator=(odWindow&& other);
	OD_ENGINE_PLATFORM_MODULE ~odWindow();

	odWindow(odWindow const& other) = delete;
	odWindow& operator=(const odWindow& other) = delete;
};
