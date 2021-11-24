#pragma once

#include <od/engine/world.h>

#include <od/core/array.hpp>

struct odEntity;

struct odEntityStorage;

OD_ENGINE_MODULE extern template struct odArrayT<odEntity>;

OD_ENGINE_MODULE extern template struct odArrayT<odEntityStorage>;

struct odWorld {
	odArrayT<odEntityStorage> entities;
	odArrayT<odEntity> chunks[OD_WORLD_CHUNK_ID_MAX];

	OD_ENGINE_MODULE odWorld();
	OD_ENGINE_MODULE odWorld(odWorld&& other);
	OD_ENGINE_MODULE odWorld& operator=(odWorld&& other);
	OD_ENGINE_MODULE ~odWorld();

	odWorld(const odWorld& other) = delete;
	odWorld& operator=(const odWorld& other) = delete;
};
