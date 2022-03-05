#pragma once

#include <od/platform/window.h>

#include <od/core/array.hpp>

struct odWindow {
	odWindowSettings settings;
	void* window_native;
	void* render_context_native;
	bool is_sdl_init;
	bool is_open;
	int32_t next_frame_ms;

	odArrayT<odWindowResource*> resources;

	OD_PLATFORM_MODULE odWindow();
	OD_PLATFORM_MODULE odWindow(odWindow&& other);
	OD_PLATFORM_MODULE odWindow& operator=(odWindow&& other);
	OD_PLATFORM_MODULE ~odWindow();

	odWindow(odWindow const& other) = delete;
	odWindow& operator=(const odWindow& other) = delete;
};
struct odWindowResource {
	odWindow* window;

	OD_PLATFORM_MODULE odWindowResource();
	OD_PLATFORM_MODULE odWindowResource(odWindowResource&& other);
	OD_PLATFORM_MODULE odWindowResource& operator=(odWindowResource&& other);
	OD_PLATFORM_MODULE ~odWindowResource();

	odWindowResource(odWindowResource const& other) = delete;
	odWindowResource& operator=(const odWindowResource& other) = delete;
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
