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
	int32_t* out_entity_ids;
	int32_t max_results;
	struct odBounds bounds;
	struct odTagset tagset;
	const int32_t* opt_excluded_entity_id;
};

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const char*
odWorldSearch_get_debug_string(const struct odWorldSearch* search);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorldSearch_check_valid(const struct odWorldSearch* search);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorldSearch_matches_entity(const struct odWorldSearch* search, const struct odEntity* entity);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_init(struct odWorld* world);
OD_API_C OD_ENGINE_MODULE void
odWorld_destroy(struct odWorld* world);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_set(struct odWorld* world, const struct odEntity* entity);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_get(const struct odWorld* world, int32_t entity_id, struct odEntity* out_entity);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_set_sprite(struct odWorld* world, int32_t entity_id, const struct odEntitySprite* sprite);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_get_sprite(const struct odWorld* world, int32_t entity_id, struct odEntitySprite* out_sprite);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD int32_t
odWorld_search(const struct odWorld* world, const struct odWorldSearch* search);
