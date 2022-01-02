#pragma once

#include <od/engine/module.h>

#include <od/core/bounds.h>
#include <od/engine/tagset.h>
#include <od/engine/entity.h>

// input parameters to the chunk indexing system, can be tweaked at compile-time as needed per game:
#define OD_ENTITY_CHUNK_OPTIMUM_WORLD_WIDTH_BITS 9  // 2^9 = 512; optimal game world <= 512x512 pixels
#define OD_ENTITY_CHUNK_OPTIMUM_CHUNK_WIDTH_BITS 4  // 2^4 = 16; game world split into 16x16 pixel chunks

#define OD_ENTITY_CHUNK_COORD_COUNT 2  // x, y
#define OD_ENTITY_CHUNK_COORD_DISCARD_BITS (OD_ENTITY_CHUNK_OPTIMUM_CHUNK_WIDTH_BITS)
#define OD_ENTITY_CHUNK_COORD_MASK_BITS \
	(OD_ENTITY_CHUNK_OPTIMUM_WORLD_WIDTH_BITS - OD_ENTITY_CHUNK_COORD_DISCARD_BITS)
#define OD_ENTITY_CHUNK_COORD_BITS \
	((OD_ENTITY_CHUNK_COORD_DISCARD_BITS) + (OD_ENTITY_CHUNK_COORD_MASK_BITS))
#define OD_ENTITY_CHUNK_ID_BITS ((OD_ENTITY_CHUNK_COORD_MASK_BITS) * (OD_ENTITY_CHUNK_COORD_COUNT))
#define OD_ENTITY_CHUNK_ID_COUNT (1 << (OD_ENTITY_CHUNK_ID_BITS))

struct odVertex;

struct odEntityIndex;

struct odEntitySearch {
	odEntityId* out_results;
	int32_t max_results;
	struct odBounds bounds;
	struct odTagset tagset;
};

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const char*
odEntityIndex_get_debug_string(const struct odEntityIndex* entity_index);
OD_API_C OD_ENGINE_MODULE void
odEntityIndex_init(struct odEntityIndex* entity_index);
OD_API_C OD_ENGINE_MODULE void
odEntityIndex_destroy(struct odEntityIndex* entity_index);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD int32_t
odEntityIndex_get_count(const struct odEntityIndex* entity_index);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const struct odVertex*
odEntityIndex_get_vertices(const struct odEntityIndex* entity_index, odEntityId entity_id);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const struct odVertex*
odEntityIndex_get_all_vertices(const struct odEntityIndex* entity_index, int32_t* out_vertex_count);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const struct odEntity*
odEntityIndex_get(const struct odEntityIndex* entity_index, odEntityId entity_id);
OD_API_C OD_ENGINE_MODULE void
odEntityIndex_set_collider(struct odEntityIndex* entity_index, const struct odEntityCollider* collider);
OD_API_C OD_ENGINE_MODULE void
odEntityIndex_set_sprite(struct odEntityIndex* entity_index, odEntityId entity_id, const struct odEntitySprite* sprite);
OD_API_C OD_ENGINE_MODULE void
odEntityIndex_set(struct odEntityIndex* entity_index, const struct odEntity* entity);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD /*num_results*/ int32_t
odEntityIndex_search(const struct odEntityIndex* entity_index, const struct odEntitySearch* search);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const char*
odEntitySearch_get_debug_string(const struct odEntitySearch* search);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntitySearch_check_valid(const struct odEntitySearch* search);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntitySearch_matches_collider(const struct odEntitySearch* search, const struct odEntityCollider* collider);
