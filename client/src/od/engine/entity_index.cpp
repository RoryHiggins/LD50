#pragma once

#include <od/engine/entity_index.hpp>

#include <od/core/container.hpp>
#include <od/core/primitive.h>

#include <cmath>

#define OD_ENTITY_TAGS_COUNT 64
#define OD_ENTITY_TAGS_SIZE ((OD_ENTITY_TAGS_COUNT + 7) / 8)
#define OD_ENTITY_CHUNK_COUNT (1 << (2 * OD_ENTITY_CHUNK_COORD_BITS))

struct odEntityTags {
	uint8_t bits[OD_ENTITY_TAGS_SIZE];
};

struct odEntityBounds {
	int16_t x;
	int16_t y;
	int16_t width;
	int16_t height;
};

struct odEntityTransform {
	int16_t translate_x;
	int16_t translate_y;
	int16_t scale_x;
	int16_t scale_y;
	int16_t rotate;
};

struct odEntity {
	odEntityTags tags;
	odEntityBounds bounds;

	float depth;
	odColor color;

	odBounds texture_bounds;
	odTransform render_transform;
	odVertex render_vertices[6];
};
struct odEntityChunkEntity {
	int32_t entity_id;
	uint8_t chunk_relative_x;
	uint8_t chunk_relative_y;
	uint8_t chunk_relative_w;
	uint8_t chunk_relative_h;
};
struct odWorld {
	int32_t entity_count;

	odArrayT<odEntity> entities;  // by entity id
	odArrayT<odArrayT<int32_t>> entities_by_tag_id;  // entity ids by tag id
	odArrayT<odEntity> entities_by_chunk[65536];  // by chunk id
};
uint16_t odWorld_get_chunk(int16_t x, int16_t y);
uint16_t odWorld_get_chunk(int16_t x, int16_t y) {
	return (
		static_cast<uint16_t>(static_cast<uint8_t>(x >> 8))
		+ static_cast<uint16_t>(static_cast<uint16_t>(static_cast<uint8_t>(y >> 8)) << 8)
	);
}
