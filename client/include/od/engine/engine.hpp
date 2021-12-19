#pragma once

#include <od/engine/engine.h>

#include <od/core/trivial_array.hpp>
#include <od/core/vertex.hpp>
#include <od/platform/window.hpp>
#include <od/platform/texture.hpp>
#include <od/platform/renderer.hpp>
#include <od/engine/entity_index.hpp>

struct odEngineFrame {
	int32_t counter;
	odTrivialArrayT<odVertex> game_vertices;
	odTrivialArrayT<odVertex> window_vertices;

	odEngineFrame();
};

struct odEngine {
	odEngineSettings settings;

	odWindow window;
	odRenderer renderer;
	odTexture src_texture;
	odRenderTexture game_render_texture;
	bool is_initialized;

	odEntityIndex entity_index;
	odEngineFrame frame;

	OD_ENGINE_MODULE odEngine();
	OD_ENGINE_MODULE odEngine(odEngine&& other);
	OD_ENGINE_MODULE odEngine& operator=(odEngine&& other);
	OD_ENGINE_MODULE ~odEngine();

	odEngine(const odEngine& other) = delete;
	odEngine& operator=(const odEngine& other) = delete;
};
