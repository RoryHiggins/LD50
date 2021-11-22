#pragma once

#include <od/engine/entity.hpp>

#include <cmath>

#include <od/core/debug.h>
#include <od/core/container.hpp>
#include <od/core/primitive.h>

#define OD_ENTITY_TAGS_SIZE 32
#define OD_ENTITY_TAGS_COUNT (8 * OD_ENTITY_TAGS_SIZE)

#define OD_ENTITY_CHUNK_COORD_DROPPED_BITS 4
#define OD_ENTITY_CHUNK_COORD_BITS 4
#define OD_ENTITY_CHUNK_COUNT (1 << (2 * OD_ENTITY_CHUNK_COORD_BITS))

typedef uint8_t odTagId;
typedef int32_t odEntityId;

int32_t odWorld_get_chunk_coord(float coord);
int32_t odWorld_get_chunk(float x, float y);

struct odEntityTags {
	uint8_t bytes[OD_ENTITY_TAGS_SIZE];
};
struct odEntity {
	odEntityId id;
	odBounds bounds;
	odEntityTags tags;

	odBounds texture_bounds;
	odTransform transform;
	odColor color;
	float depth;
};
struct odChunkEntity {
	odEntityId id;
	odBounds bounds;
	odEntityTags tags;
};

struct odWorld {
	odArrayT<odEntity> entities;
	odArrayT<odChunkEntity> chunk_entities[OD_ENTITY_CHUNK_COUNT];
};


int32_t odWorld_get_chunk_coord(float coord) {
	const int32_t coord_bitmask = (1 << OD_ENTITY_CHUNK_COORD_BITS) - 1;
	return (static_cast<int32_t>(coord) >> OD_ENTITY_CHUNK_COORD_DROPPED_BITS) & coord_bitmask;
}
int32_t odWorld_get_chunk(float x, float y) {
	return odWorld_get_chunk_coord(x) + (odWorld_get_chunk_coord(y) << OD_ENTITY_CHUNK_COORD_BITS);
}


// struct odRenderWorld {
// 	odArrayT<odSpriteComponent> sprites;
// 	odArrayT<odVertexComponent> vertices;
// };


/*

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
