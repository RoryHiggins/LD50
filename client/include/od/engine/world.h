#pragma once

#include <od/engine/module.h>

#include <od/engine/entity.h>

#define OD_WORLD_CHUNK_COORD_DISCARD_BITS 4
#define OD_WORLD_CHUNK_COORD_MASK_BITS 4
#define OD_WORLD_CHUNK_COORD_COUNT 2  // x, y
#define OD_WORLD_CHUNK_COUNT \
	(1 << (OD_WORLD_CHUNK_COORD_MASK_BITS * OD_WORLD_CHUNK_COORD_COUNT))

struct odTagset;
struct odBounds;

struct odWorld;

struct odWorldSearch {
	const struct odTagset* opt_tags;
	const struct odBounds* opt_bounds;
	const odEntityIndex* opt_excluded_entity_index;
};
struct odWorldIterator {
	struct odWorldSearch search;

	int32_t chunk_id;
	int32_t index;
};

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_init(struct odWorld* world);
OD_API_C OD_ENGINE_MODULE void
odWorld_destroy(struct odWorld* world);
OD_API_C OD_ENGINE_MODULE void
odWorld_unset(struct odWorld* world, odEntityIndex entity_index);
OD_API_C OD_ENGINE_MODULE void
odWorld_set(struct odWorld* world, odEntityIndex entity_index, const struct odEntity* entity);
OD_API_C OD_ENGINE_MODULE void
odWorld_set_bounds(struct odWorld* world, odEntityIndex entity_index, const struct odBounds* bounds);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_exists(const struct odWorld* world, odEntityIndex entity_index);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const struct odEntity*
odWorld_get(const struct odWorld* world, odEntityIndex entity_index);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD odEntityIndex
odWorld_search_init(const struct odWorld* world, const struct odWorldSearch* search,
					struct odWorldIterator* out_iterator);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odWorld_search(const struct odWorld* world, struct odWorldIterator* iterator,
			   odEntityIndex* out_entity_index);
