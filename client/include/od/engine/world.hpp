#pragma once

#include <od/engine/world.h>

#include <od/core/array.hpp>

struct odWorld {
	odArrayT<odEntity> entities;
	odArrayT<odEntityKey> entities_by_chunk[OD_WORLD_CHUNK_COUNT];

	OD_ENGINE_MODULE odWorld();
	OD_ENGINE_MODULE odWorld(odWorld&& other);
	OD_ENGINE_MODULE odWorld& operator=(odWorld&& other);
	OD_ENGINE_MODULE ~odWorld();

	odWorld(const odWorld& other) = delete;
	odWorld& operator=(const odWorld& other) = delete;
};
