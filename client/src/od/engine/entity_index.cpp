#pragma once

#include <od/engine/entity_index.hpp>

#include <od/core/container.hpp>
#include <od/core/primitive.h>

#define OD_ENTITY_CHUNK_BOUNDS_SOA_COUNT 8

#define OD_TAG_INLINE_COUNT 16

#define OD_ENTITY_CHUNK_SIZE 128
#define OD_ENTITY_CHUNK_BITS 6  // float max consecutive integer bits
#define OD_ENTITY_CHUNK_MASK ((1 << (OD_ENTITY_CHUNK_BITS)) - 1)

struct odEntity {
	int32_t tag_entities_index_id_inline[OD_TAG_INLINE_COUNT];
	int32_t tags_inline[OD_TAG_INLINE_COUNT];  // reduce allocations: use if tag_count <= OD_TAG_INLINE_COUNT

	int32_t* tags;
	int32_t* tag_entities_index_id;
	int32_t tag_count;

	odBounds bounds;
	float depth;
	odColor color;
	odBounds texture_bounds;
	odTransform render_transform;

	odVertex render_vertices[6];
};
struct odEntityChunkEntitySoA {
	int32_t entity_id[OD_ENTITY_CHUNK_BOUNDS_SOA_COUNT];

	int32_t x[OD_ENTITY_CHUNK_BOUNDS_SOA_COUNT];
	int32_t y[OD_ENTITY_CHUNK_BOUNDS_SOA_COUNT];
	int32_t width[OD_ENTITY_CHUNK_BOUNDS_SOA_COUNT];
	int32_t height[OD_ENTITY_CHUNK_BOUNDS_SOA_COUNT];

	// potential future feature: topmost depth collision, for GUIs
	// float depth[OD_ENTITY_CHUNK_BOUNDS_SOA_COUNT];

	// potential optimization to avoid an indirection:
	// int32_t tag_count[OD_ENTITY_CHUNK_BOUNDS_SOA_COUNT];
	// int32_t* tags[OD_ENTITY_CHUNK_BOUNDS_SOA_COUNT];  // not owned
};
struct odEntityChunk {
	int32_t entity_count;
	odArrayT<odEntityChunkEntitySoA> entities;
};
struct odEntityIndex {
	int32_t entity_count;

	odArrayT<odEntity> entities;  // by entity id

	odArrayT<odArrayT<int32_t>> tag_entities;  // entity ids by tag id

	// chunk = (((x / SIZE) % MASK) << BITS) + ((y / SIZE) % MASK)
	odArrayT<odArrayT<odEntityChunk>> chunks;  // by chunk id
};
