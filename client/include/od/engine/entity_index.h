#pragma once

#include <od/engine/module.h>

struct odBounds;
struct odTransform;
struct odColor;
struct odArray;

struct odEntityIndex;

struct odEntitySearch {
	struct odArray* opt_out_entity_ids;
	int32_t* opt_out_count;
	bool* opt_out_any;

	int32_t opt_tag_count;
	const int32_t* opt_tags;

	const struct odBounds* opt_bounds;

	const int32_t* opt_exclude_entity_id;
};
struct odEntityUpdate {
	const int32_t* entity_ids;

	const int32_t* opt_tags;

	const struct odBounds* opt_bounds;
	const float* opt_depth;

	const struct odColor* opt_color;
	const struct odBounds* opt_texture_bounds;
	const struct odTransform* opt_render_transform;
};

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD
bool odEntityIndex_init(struct odEntityIndex* world);

OD_API_C OD_ENGINE_MODULE
void odEntityIndex_destroy(struct odEntityIndex* world);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD
bool odEntityIndex_ensure_capacity(struct odEntityIndex* world, int32_t min_entity_capacity);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD
int32_t odEntityIndex_entity_clear(struct odEntityIndex* world, int32_t entity_id);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD
bool odEntityIndex_entity_search(const struct odEntityIndex* world, struct odEntitySearch search);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD
bool odEntityIndex_entity_update(struct odEntityIndex* world, struct odEntityUpdate update);
