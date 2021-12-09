#pragma once

#include <od/engine/engine.h>

#include <od/core/fast_array.hpp>
#include <od/platform/window.hpp>
#include <od/platform/vertex.hpp>
#include <od/engine/tagset.h>
#include <od/engine/entity_index.hpp>

struct odEngineFrame {
	int32_t counter;
	odFastArrayT<odVertex> game_vertices;
	odFastArrayT<odVertex> window_vertices;

	odEngineFrame();
};

struct odEngine {
	bool is_initialized;
	odWindow window;
	odTagNames tag_names;
	odEntityIndex entity_index;
	odEngineFrame frame;

	OD_ENGINE_MODULE odEngine();
	OD_ENGINE_MODULE odEngine(odEngine&& other);
	OD_ENGINE_MODULE odEngine& operator=(odEngine&& other);
	OD_ENGINE_MODULE ~odEngine();

	odEngine(const odEngine& other) = delete;
	odEngine& operator=(const odEngine& other) = delete;
};
