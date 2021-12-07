#pragma once

#include <od/engine/engine.h>

#include <od/platform/window.hpp>
#include <od/engine/tagset.h>
#include <od/engine/entity_index.hpp>
#include <od/engine/lua_client.hpp>

struct odEngine {
	bool is_initialized;
	odWindow window;
	odTagNames tag_names;
	odEntityIndex entity_index;
	odLuaClient lua_client;

	OD_ENGINE_MODULE odEngine();
	OD_ENGINE_MODULE odEngine(odEngine&& other);
	OD_ENGINE_MODULE odEngine& operator=(odEngine&& other);
	OD_ENGINE_MODULE ~odEngine();

	odEngine(const odEngine& other) = delete;
	odEngine& operator=(const odEngine& other) = delete;
};
