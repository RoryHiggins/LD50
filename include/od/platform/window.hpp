#pragma once

#include <od/platform/window.h>

struct odWindow {
	void* window_native;
	void* render_context_native;
	bool is_open;
	uint32_t next_frame_ms;
	struct odWindowSettings settings;

	OD_API_PLATFORM_CPP odWindow();
	OD_API_PLATFORM_CPP odWindow(odWindow&& other);
	OD_API_PLATFORM_CPP odWindow& operator=(odWindow&& other);
	OD_API_PLATFORM_CPP ~odWindow();

	odWindow(odWindow const& other) = delete;
	odWindow& operator=(const odWindow& other) = delete;
};
