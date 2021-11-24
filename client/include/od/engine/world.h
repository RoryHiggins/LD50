#pragma once

#include <od/engine/module.h>

#include <od/core/bounds.h>
#include <od/engine/tagset.h>

#define OD_WORLD_CHUNK_COORD_DISCARD_BITS 4
#define OD_WORLD_CHUNK_COORD_MASK_BITS 4
#define OD_WORLD_CHUNK_COORD_COUNT 2  // x, y
#define OD_WORLD_CHUNK_ID_MAX \
	(1 << (OD_WORLD_CHUNK_COORD_MASK_BITS * OD_WORLD_CHUNK_COORD_COUNT))

struct odEntity;
struct odEntitySprite;

struct odWorld;

struct odWorldSearch {
	struct odBounds bounds;
	struct odTagset tagset;
	int32_t excluded_entity_id; // -1 = do not exclude any valid entity

	uint8_t current_chunk_x;
	uint8_t current_chunk_y;
	int32_t current_chunk_entities_index;
};

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_init(struct odWorld* world);
OD_API_C OD_ENGINE_MODULE void
odWorld_destroy(struct odWorld* world);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_entity_set(struct odWorld* world, int32_t entity_id, const struct odEntity* new_entity);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_entity_get(const struct odWorld* world, int32_t entity_id, struct odEntity* out_entity);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_entity_set_sprite(struct odWorld* world, int32_t entity_id, const struct odEntitySprite* new_sprite);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_entity_get_sprite(const struct odWorld* world, int32_t entity_id, struct odEntitySprite* out_sprite);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorldSearch_init(struct odWorldSearch* search, const struct odBounds* bounds, const struct odTagset* opt_tagset, const int32_t* opt_excluded_entity_id);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorldSearch_get_complete(const struct odWorldSearch* search);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorldSearch_next(struct odWorldSearch* search, const struct odWorld* world, int32_t* out_entity_id);
