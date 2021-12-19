#pragma once

#include <od/engine/entity_index.h>

#include <od/core/array.hpp>
#include <od/core/vertex.h>
#include <od/engine/entity.hpp>

#define OD_ENTITY_VERTEX_COUNT (OD_RECT_PRIMITIVE_VERTEX_COUNT)

struct odEntityStorage;

struct odEntityChunk {
	odTrivialArrayT<odEntityCollider> colliders;
};

OD_ENGINE_MODULE extern template struct odTrivialArrayT<odEntityStorage>;

struct odEntityIndex {
	odTrivialArrayT<odEntityStorage> entities;
	odTrivialArrayT<odVertex> entity_vertices;
	odEntityChunk chunks[OD_ENTITY_CHUNK_ID_COUNT];

	OD_ENGINE_MODULE odEntityIndex();
	OD_ENGINE_MODULE odEntityIndex(odEntityIndex&& other);
	OD_ENGINE_MODULE odEntityIndex& operator=(odEntityIndex&& other);
	OD_ENGINE_MODULE ~odEntityIndex();

	odEntityIndex(const odEntityIndex& other) = delete;
	odEntityIndex& operator=(const odEntityIndex& other) = delete;
};
