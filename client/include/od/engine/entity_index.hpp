#pragma once

#include <od/engine/entity_index.h>

#include <od/core/fast_array.hpp>
#include <od/core/array.hpp>
#include <od/engine/entity.hpp>

struct odEntityIndex {
	odArrayT<odEntity> entities;
	odFastArrayT<odEntityCollider> chunks[OD_WORLD_CHUNK_ID_MAX];

	OD_ENGINE_MODULE odEntityIndex();
	OD_ENGINE_MODULE odEntityIndex(odEntityIndex&& other);
	OD_ENGINE_MODULE odEntityIndex& operator=(odEntityIndex&& other);
	OD_ENGINE_MODULE ~odEntityIndex();

	odEntityIndex(const odEntityIndex& other) = delete;
	odEntityIndex& operator=(const odEntityIndex& other) = delete;
};
