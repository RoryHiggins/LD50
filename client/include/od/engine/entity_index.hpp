#pragma once

#include <od/engine/entity_index.h>

#include <od/core/array.hpp>
#include <od/core/vertex.h>
#include <od/engine/entity.hpp>

#define OD_ENTITY_VERTEX_COUNT (OD_SPRITE_VERTEX_COUNT)

struct odEntityIndexEntity;

struct odEntityChunk {
	odTrivialArrayT<odEntityCollider> colliders;
	odTrivialArrayT<odEntityId> entity_ids;
};

struct odEntityIndex {
	odTrivialArrayT<odEntityIndexEntity> entities;
	odTrivialArrayT<odVertex> entity_vertices;
	odEntityChunk chunks[OD_ENTITY_CHUNK_ID_COUNT];

	OD_ENGINE_MODULE odEntityIndex();
	OD_ENGINE_MODULE odEntityIndex(odEntityIndex&& other);
	OD_ENGINE_MODULE odEntityIndex& operator=(odEntityIndex&& other);
	OD_ENGINE_MODULE ~odEntityIndex();

	odEntityIndex(const odEntityIndex& other) = delete;
	odEntityIndex& operator=(const odEntityIndex& other) = delete;
};
