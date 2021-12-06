#pragma once

#include <od/engine/module.h>

#include <od/core/bounds.h>
#include <od/engine/tagset.h>

#define OD_WORLD_CHUNK_OPTIMUM_TILE_WIDTH_BITS 3  // 8x8 sized tiles
#define OD_WORLD_CHUNK_OPTIMUM_SCREEN_WIDTH_BITS 8  // 256x256 pixels

#define OD_WORLD_CHUNK_COORD_DISCARD_BITS (OD_WORLD_CHUNK_OPTIMUM_TILE_WIDTH_BITS)
#define OD_WORLD_CHUNK_COORD_MASK_BITS (OD_WORLD_CHUNK_OPTIMUM_SCREEN_WIDTH_BITS - OD_WORLD_CHUNK_OPTIMUM_TILE_WIDTH_BITS)
#define OD_WORLD_CHUNK_COORD_BITS ((OD_WORLD_CHUNK_COORD_DISCARD_BITS) + (OD_WORLD_CHUNK_COORD_MASK_BITS))
#define OD_WORLD_CHUNK_COORD_COUNT 2  // x, y
#define OD_WORLD_CHUNK_ID_MAX \
	(1 << ((OD_WORLD_CHUNK_COORD_MASK_BITS) * (OD_WORLD_CHUNK_COORD_COUNT)))

struct odEntity;
struct odEntitySprite;

struct odEntityIndex;

struct odEntitySearch {
	int32_t* out_entity_ids;
	int32_t max_entity_ids;
	struct odBounds bounds;
	struct odTagset tagset;
};

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const char*
odEntityIndex_get_debug_string(const struct odEntityIndex* entity_index);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntityIndex_init(struct odEntityIndex* entity_index);
OD_API_C OD_ENGINE_MODULE void
odEntityIndex_destroy(struct odEntityIndex* entity_index);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const struct odEntity*
odEntityIndex_get(const struct odEntityIndex* entity_index, int32_t entity_id);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntityIndex_set_collider(struct odEntityIndex* entity_index, const struct odEntityCollider* collider);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntityIndex_set_sprite(struct odEntityIndex* entity_index, int32_t entity_id, const struct odEntitySprite* sprite);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntityIndex_set(struct odEntityIndex* entity_index, const struct odEntity* entity);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD int32_t
odEntityIndex_search(const struct odEntityIndex* entity_index, const struct odEntitySearch* search);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const char*
odEntitySearch_get_debug_string(const struct odEntitySearch* search);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntitySearch_check_valid(const struct odEntitySearch* search);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntitySearch_matches_collider(const struct odEntitySearch* search, const struct odEntityCollider* collider);
