#pragma once

#include <od/engine/engine.h>

#include <od/platform/window.hpp>

struct odEngine {
	odWindow window;
	bool is_initialized;

	OD_ENGINE_MODULE odEngine();
	OD_ENGINE_MODULE odEngine(odEngine&& other);
	OD_ENGINE_MODULE odEngine& operator=(odEngine&& other);
	OD_ENGINE_MODULE ~odEngine();

	odEngine(const odEngine& other) = delete;
	odEngine& operator=(const odEngine& other) = delete;
};
