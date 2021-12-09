#pragma once

#include <od/engine/entity_index.h>

#include <od/core/fast_array.hpp>
#include <od/core/array.hpp>
#include <od/platform/vertex.h>
#include <od/engine/entity.hpp>

#define OD_ENTITY_VERTEX_COUNT (OD_PRIMITIVE_RECT_VERTEX_COUNT)

struct odEntityStorage;

struct odEntityChunk {
	odFastArrayT<odEntityCollider> colliders;
};

OD_ENGINE_MODULE extern template struct odFastArrayT<odEntityStorage>;

struct odEntityIndex {
	odFastArrayT<odEntityStorage> entities;
	odFastArrayT<odVertex> entity_vertices;
	odEntityChunk chunks[OD_ENTITY_CHUNK_ID_COUNT];

	OD_ENGINE_MODULE odEntityIndex();
	OD_ENGINE_MODULE odEntityIndex(odEntityIndex&& other);
	OD_ENGINE_MODULE odEntityIndex& operator=(odEntityIndex&& other);
	OD_ENGINE_MODULE ~odEntityIndex();

	odEntityIndex(const odEntityIndex& other) = delete;
	odEntityIndex& operator=(const odEntityIndex& other) = delete;
};
