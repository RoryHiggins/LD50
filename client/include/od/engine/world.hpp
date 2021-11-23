#pragma once

#include <od/engine/world.h>

#include <od/core/array.hpp>
#include <od/engine/tagset.h>
#include <od/engine/entity.hpp>

struct odEntityStorage;

OD_ENGINE_MODULE extern template struct odArrayT<odEntityStorage>;

struct odWorld {
	odArrayT<odEntityStorage> entity_storage;
	odArrayT</*entity_index*/ int32_t> chunk_entity_map[OD_WORLD_CHUNK_INDEX_MAX];
	odArrayT</*entity_index*/ int32_t> tag_entity_map[OD_TAG_INDEX_MAX];

	OD_ENGINE_MODULE odWorld();
	OD_ENGINE_MODULE odWorld(odWorld&& other);
	OD_ENGINE_MODULE odWorld& operator=(odWorld&& other);
	OD_ENGINE_MODULE ~odWorld();

	odWorld(const odWorld& other) = delete;
	odWorld& operator=(const odWorld& other) = delete;
};
