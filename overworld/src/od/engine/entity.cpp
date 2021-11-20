#pragma once

#include <od/engine/entity.hpp>

#include <od/core/debug.h>
#include <od/core/container.hpp>
#include <od/core/primitive.h>

#define OD_ENTITY_TAGS_SIZE 4
#define OD_ENTITY_TAGS_COUNT (8 * OD_ENTITY_TAGS_SIZE)

#define OD_WORLD_CHUNK_ENTITY_SOA_COUNT 8

typedef uint8_t odTagId;
typedef int32_t odEntityId;
typedef int32_t odEntityHandle;

struct odEntityTags {
	uint8_t bytes[OD_ENTITY_TAGS_SIZE];
};
struct odEntityBounds {
	int16_t x1;
	int16_t y1;
	int16_t x2;
	int16_t y2;
};
struct odEntity {
	odEntityId id;
	odEntityBounds bounds;
	odEntityTags tags;
};

struct odWorldChunkEntitySoA {
	odEntityId id[OD_WORLD_CHUNK_ENTITY_SOA_COUNT];
	int16_t x1[OD_WORLD_CHUNK_ENTITY_SOA_COUNT];
	int16_t y1[OD_WORLD_CHUNK_ENTITY_SOA_COUNT];
	int16_t x2[OD_WORLD_CHUNK_ENTITY_SOA_COUNT];
	int16_t y2[OD_WORLD_CHUNK_ENTITY_SOA_COUNT];
};
struct odWorldChunk {
	odArrayT<odEntity> entities;
};





struct odEntityTexture {
	int16_t u1;
	int16_t v1;
	int16_t u2;
	int16_t v2;
};
struct odEntitySpriteComponent {
	odEntityTexture texture;
	odTransform transform;
	odColor color;
	float depth;
	int32_t vertex_index;
};
struct odEntityVertexCache {
	float depth;
	odVertex vertices[6];
};
// struct odRenderWorld {
// 	odArrayT<odSpriteComponent> sprites;
// 	odArrayT<odVertexComponent> vertices;
// };


/*

#define OD_ENTITY_CHUNK_COUNT 65536

struct odEntity {
	odEntityId id;
	odEntityTags tags;
	odBounds bounds;
	odBounds texture;
	odColor color;
	float depth;
};
struct odEntityVertexCache {
	odVertex vertices[6];
};
struct odEntityTagIndexCache {
	int32_t tag_index[OD_ENTITY_TAGS_COUNT];  // by tag id
};
struct odWorldChunkEntity {
	odBounds bounds;
	odEntityId id;
};
struct odWorldChunk {
	odArrayT<odWorldChunkEntity> entities;
};
struct odWorld {
	int32_t entity_count;

	odArrayT<odEntity> entities;  // by entity index
	odArrayT<int32_t> free_entity_indices;  // entity indices
	odArrayT<odEntityVertexCache> entity_vertex_cache;  // by entity index
	odArrayT<odEntityTagIndexCache> entity_tag_index_cache;  // by entity index
	odArrayT<odArrayT<odEntityId>> entities_by_tag_id;  // by tag id
	odWorldChunk chunks[OD_ENTITY_CHUNK_COUNT];  // by chunk id
};

bool odEntityTags_has_tag(odEntityTags tags, odTagId tag_id);
OD_NO_DISCARD
odEntityTags odEntityTags_update_tag(odEntityTags tags, odTagId tag_id, bool enabled);

int32_t odEntityId_get_index(odEntityId id);
int32_t odEntityId_get_generation(odEntityId id);

uint16_t odWorld_get_chunk(int16_t x, int16_t y);

bool odEntityTags_has_tag(odEntityTags tags, odTagId tag_id) {
	if (!OD_DEBUG_CHECK(tag_id <= OD_ENTITY_TAGS_COUNT)) {
		return false;
	}

	uint8_t byte = tag_id >> 3;
	uint8_t bit = tag_id & 7;
	uint8_t bit_mask = static_cast<uint8_t>(1 << bit);

	return (tags.bytes[byte] & bit_mask) > 0;
}
odEntityTags odEntityTags_update_tag(odEntityTags tags, odTagId tag_id, bool enabled) {
	if (!OD_DEBUG_CHECK(tag_id <= OD_ENTITY_TAGS_COUNT)) {
		return tags;
	}

	uint8_t byte = tag_id >> 3;
	uint8_t bit = tag_id & 7;
	uint8_t bit_mask = static_cast<uint8_t>(1 << bit);
	uint8_t enabled_bit_mask = static_cast<uint8_t>(enabled << bit);
	
	tags.bytes[byte] = (tags.bytes[byte] ^ bit_mask) | enabled_bit_mask;
	return tags;
}

int32_t odEntityId_get_index(odEntityId id) {
	return id >> 8;
}
int32_t odEntityId_get_generation(odEntityId id) {
	return id & 255;
}

uint16_t odWorld_get_chunk(int16_t x, int16_t y) {
	return (
		static_cast<uint16_t>(static_cast<uint8_t>(x >> 8))
		+ static_cast<uint16_t>(static_cast<uint16_t>(static_cast<uint8_t>(y >> 8)) << 8)
	);
}
*/
