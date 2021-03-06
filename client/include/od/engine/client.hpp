#pragma once

#include <od/engine/client.h>

#include <od/core/array.hpp>
#include <od/core/vertex.h>
#include <od/platform/window.hpp>
#include <od/platform/texture.hpp>
#include <od/platform/render_texture.hpp>
#include <od/platform/renderer.hpp>
#include <od/engine/entity_index.hpp>

struct odClientFrame {
	int32_t counter;
	odTrivialArrayT<odVertex> game_vertices;
	odTrivialArrayT<odVertex> window_vertices;

	OD_ENGINE_MODULE odClientFrame();
	OD_ENGINE_MODULE odClientFrame(odClientFrame&& other);
	OD_ENGINE_MODULE odClientFrame(const odClientFrame& other);
	OD_ENGINE_MODULE ~odClientFrame();
	OD_ENGINE_MODULE odClientFrame& operator=(odClientFrame&& other);
	OD_ENGINE_MODULE odClientFrame& operator=(const odClientFrame& other);
};

struct odClient {
	odClientSettings settings;

	odWindow window;
	odRenderer renderer;
	odTexture src_texture;
	odRenderTexture game_render_texture;
	bool is_initialized;

	odEntityIndex entity_index;
	odClientFrame frame;

	OD_ENGINE_MODULE odClient();
	OD_ENGINE_MODULE odClient(odClient&& other);
	OD_ENGINE_MODULE odClient& operator=(odClient&& other);
	OD_ENGINE_MODULE ~odClient();

	odClient(const odClient& other) = delete;
	odClient& operator=(const odClient& other) = delete;
};
