#pragma once

#include <od/engine/world.hpp>

#include <cstdio>

#include <od/core/bounds.h>
#include <od/engine/tagset.h>
#include <od/engine/entity.hpp>

struct odEntityStorage {
	odEntity entity;
	odEntitySprite sprite;
};

struct odChunkIterator {
	uint8_t x1;
	uint8_t y1;
	uint8_t x2;
	uint8_t y2;

	explicit odChunkIterator(const odBounds& bounds);

	odChunkIterator begin() const;
	odChunkIterator end() const;
	bool operator==(const odChunkIterator& other) const;
	bool operator!=(const odChunkIterator& other) const;
	odChunkIterator& operator++();
	odChunkIterator& operator++(int);
	/*chunk_id*/ int32_t operator*();
};

static int32_t odChunkIterator_contains_coords(const odChunkIterator* chunk_iterator, uint8_t x, uint8_t y);
static int32_t odChunkIterator_contains_chunk(const odChunkIterator* chunk_iterator, int32_t chunk_id);

static uint8_t odChunkCoord_init(float coord);

// static int32_t odChunkId_init(float x, float y);
static int32_t odChunkId_init_coords(uint8_t x, uint8_t y);
static uint8_t odChunkId_get_coord_x(int32_t chunk_id);
static uint8_t odChunkId_get_coord_y(int32_t chunk_id);

const char* odWorldSearch_get_debug_string(const odWorldSearch* search) {
	if (search == nullptr) {
		return "odWorldSearch{this=nullptr}";
	}

	const size_t excluded_entity_id_buffer_size = 16;
	char excluded_entity_id_str[excluded_entity_id_buffer_size] = "nullptr";
	if (search->opt_excluded_entity_id != nullptr) {
		snprintf(excluded_entity_id_str, excluded_entity_id_buffer_size, "%d", *search->opt_excluded_entity_id);
	}

	return odDebugString_format(
		"odWorldSearch{this=%p, max_results=%d, tagset=%s, bounds=%s, opt_excluded_entity_id=%s, opt_out_entity_ids=%p}",
		static_cast<const void*>(search),
		search->max_results,
		odTagset_get_debug_string(search->tagset),
		odBounds_get_debug_string(search->bounds),
		excluded_entity_id_str,
		static_cast<const void*>(search->opt_out_entity_ids));
}
bool odWorldSearch_check_valid(const odWorldSearch* search) {
	if (!OD_CHECK(search != nullptr)) {
		return false;
	}

	if (!OD_CHECK(search->max_results > 0)
		|| !OD_CHECK(search->tagset != nullptr)
		|| !OD_CHECK(odBounds_check_valid(search->bounds)
		|| !OD_CHECK((search->opt_excluded_entity_id == nullptr) || (*search->opt_excluded_entity_id >= 0)))) {
		return false;
	}

	return true;
}
bool odWorldSearch_matches_entity(const odWorldSearch* search, const odEntity* entity) {
	if (!OD_DEBUG_CHECK(odWorldSearch_check_valid(search))
		|| !OD_DEBUG_CHECK(odEntity_check_valid(entity))) {
		return false;
	}

	if (!odTagset_intersects(&entity->tagset, search->tagset)
		|| !odBounds_collides(&entity->bounds, search->bounds)
		|| ((search->opt_excluded_entity_id != nullptr) && (entity->id == *search->opt_excluded_entity_id))) {
		return false;
	}

	return true;
}

bool odWorld_init(odWorld* world) {
	OD_DEBUG("world=%p", static_cast<const void*>(world));

	if (!OD_DEBUG_CHECK(world != nullptr)) {
		return false;
	}

	odWorld_destroy(world);

	return true;
}
void odWorld_destroy(odWorld* world) {
	OD_DEBUG("world=%p", static_cast<const void*>(world));

	if (!OD_DEBUG_CHECK(world != nullptr)) {
		return;
	}

	odArray_destroy(&world->entities);

	for (int32_t i = 0; i < OD_WORLD_CHUNK_ID_MAX; i++) {
		odArray_destroy(&world->chunks[i]);
	}
}
static OD_NO_DISCARD bool odWorld_chunk_find_entity(odWorld* world, int32_t chunk_id, int32_t entity_id, int32_t* opt_out_chunk_index) {
	if (!OD_DEBUG_CHECK(world != nullptr)
		|| !OD_DEBUG_CHECK((chunk_id >= 0) && (chunk_id < OD_WORLD_CHUNK_ID_MAX))
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < odArray_get_count(&world->entities)))) {
		return false;
	}

	odArrayT<odEntity>* chunk_entity_array = &world->chunks[chunk_id];
	int32_t chunk_entity_count = odArray_get_count(chunk_entity_array);
	const odEntity* chunk_entities = world->chunks[chunk_id].begin();
	if (!OD_DEBUG_CHECK((chunk_entities != nullptr) || (chunk_entity_count == 0))) {
		return false;
	}

	int32_t chunk_index = -1;
	bool found = false;
	for (int32_t i = 0; i < chunk_entity_count; i++) {
		if (chunk_entities[i].id == entity_id) {
			chunk_index = i;
			found = true;
			break;
		}
	}

	if (opt_out_chunk_index != nullptr) {
		*opt_out_chunk_index = chunk_index;
	}

	return found;
}
static OD_NO_DISCARD bool odWorld_chunk_unset_entity(odWorld* world, int32_t chunk_id, int32_t entity_id) {
	if (!OD_DEBUG_CHECK(world != nullptr)
		|| !OD_DEBUG_CHECK((chunk_id >= 0) && (chunk_id < OD_WORLD_CHUNK_ID_MAX))
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < odArray_get_count(&world->entities)))) {
		return false;
	}

	odArrayT<odEntity>* chunk_entity_array = &world->chunks[chunk_id];
	int32_t chunk_index = -1;
	if (!OD_DEBUG_CHECK(!odWorld_chunk_find_entity(world, chunk_id, entity_id, &chunk_index))) {
		return false;
	}

	if (!OD_DEBUG_CHECK((chunk_index >= 0) && (chunk_index < odArray_get_count(chunk_entity_array)))) {
		return false;
	}

	if (!odArray_swap_pop(chunk_entity_array, chunk_index)) {
		return false;
	}

	return true;
}
static OD_NO_DISCARD bool odWorld_chunk_set_entity(odWorld* world, int32_t chunk_id, const odEntity* entity) {
	if (!OD_DEBUG_CHECK(world != nullptr)
		|| !OD_DEBUG_CHECK((chunk_id >= 0) && (chunk_id < OD_WORLD_CHUNK_ID_MAX))
		|| !OD_DEBUG_CHECK(odEntity_check_valid(entity))
		|| !OD_DEBUG_CHECK(entity->id < odArray_get_count(&world->entities))) {
		return false;
	}

	odArrayT<odEntity>* chunk_entity_array = &world->chunks[chunk_id];
	int32_t chunk_index = 0;
	if (!odWorld_chunk_find_entity(world, chunk_id, entity->id, &chunk_index)) {
		return chunk_entity_array->push(odEntity{*entity});
	}

	if (!OD_DEBUG_CHECK((chunk_index >= 0) && (chunk_index < odArray_get_count(chunk_entity_array)))) {
		return false;
	}

	odEntity* chunk_entity = (*chunk_entity_array)[chunk_index];
	if (!OD_DEBUG_CHECK(chunk_entity != nullptr)) {
		return false;
	}

	*chunk_entity = *entity;

	return true;
}
bool odWorld_set(odWorld* world, const odEntity* entity) {
	if (!OD_DEBUG_CHECK(world != nullptr)
		|| !OD_DEBUG_CHECK(odEntity_check_valid(entity))
		|| !OD_DEBUG_CHECK(entity->id < odArray_get_count(&world->entities))) {
		return false;
	}

	if (odArray_ensure_count(&world->entities, entity->id + 1)) {
		return false;
	}

	odEntityStorage* storage = world->entities[entity->id];
	if (!OD_DEBUG_CHECK(storage != nullptr)
		|| !OD_DEBUG_CHECK(odEntity_check_valid(&storage->entity))) {
		return false;
	}

	odEntity* old_entity = &storage->entity;
	if (OD_BUILD_DEBUG) {
		if (odEntity_equals(old_entity, entity)) {
			OD_WARN("Assigning entity with already assigned state, entity=%s", odEntity_get_debug_string(old_entity));
		}
	}

	odChunkIterator old_bounds_chunks{old_entity->bounds};
	odChunkIterator new_bounds_chunks{entity->bounds};
	for (int32_t chunk_id: old_bounds_chunks) {
		if (!odChunkIterator_contains_chunk(&new_bounds_chunks, chunk_id)) {
			if (!odWorld_chunk_unset_entity(world, chunk_id, entity->id)) {
				return false;
			}
		}
	}

	for (int32_t chunk_id: new_bounds_chunks) {
		if (!odWorld_chunk_set_entity(world, chunk_id, entity)) {
			return false;
		}
	}

	*old_entity = *entity;

	return true;
}
bool odWorld_get(const odWorld* world, int32_t entity_id, odEntity* out_entity) {
	if (!OD_DEBUG_CHECK(world != nullptr)
		|| !OD_DEBUG_CHECK(entity_id >= 0)
		|| !OD_DEBUG_CHECK(out_entity != nullptr)) {
		return false;
	}

	*out_entity = odEntity{};

	if (entity_id >= odArray_get_count(&world->entities)) {
		return true;
	}

	const odEntityStorage* storage = world->entities[entity_id];
	if (!OD_DEBUG_CHECK(storage != nullptr)
		|| !OD_DEBUG_CHECK(odEntity_check_valid(&storage->entity))) {
		return false;
	}

	*out_entity = storage->entity;

	return true;
}
bool odWorld_set_sprite(odWorld* world, int32_t entity_id, const odEntitySprite* sprite) {
	if (!OD_DEBUG_CHECK(world != nullptr)
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < odArray_get_count(&world->entities)))
		|| !OD_DEBUG_CHECK(odEntitySprite_check_valid(sprite))) {
		return false;
	}

	if (odArray_ensure_count(&world->entities, entity_id + 1)) {
		return false;
	}

	odEntityStorage* storage = world->entities[entity_id];
	if (!OD_DEBUG_CHECK(storage != nullptr)
		|| !OD_DEBUG_CHECK(odEntity_check_valid(&storage->entity))) {
		return false;
	}

	storage->sprite = *sprite;

	return true;
}
bool odWorld_get_sprite(const odWorld* world, int32_t entity_id, odEntitySprite* out_sprite) {
	if (!OD_DEBUG_CHECK(world != nullptr)
		|| !OD_DEBUG_CHECK(entity_id >= 0)
		|| !OD_DEBUG_CHECK(out_sprite != nullptr)) {
		return false;
	}

	*out_sprite = odEntitySprite{};

	if (entity_id >= odArray_get_count(&world->entities)) {
		return true;
	}

	const odEntityStorage* storage = world->entities[entity_id];
	if (!OD_DEBUG_CHECK(storage != nullptr)
		|| !OD_DEBUG_CHECK(odEntity_check_valid(&storage->entity))
		|| !OD_DEBUG_CHECK(odEntitySprite_check_valid(&storage->sprite))) {
		return false;
	}

	*out_sprite = storage->sprite;

	return true;
}
int32_t odWorld_search(const odWorld* world, const odWorldSearch* search) {
	if (!OD_DEBUG_CHECK(world != nullptr)
		|| !OD_DEBUG_CHECK(odWorldSearch_check_valid(search))) {
		return false;
	}

	int32_t count = 0;
	for (int32_t chunk_id: odChunkIterator{*search->bounds}) {
		for (const odEntity& entity: world->chunks[chunk_id]) {
			if (!OD_DEBUG_CHECK(odEntity_check_valid(&entity))) {
				return false;
			}

			if (!odWorldSearch_matches_entity(search, &entity)) {
				continue;
			}

			int32_t new_count = count + 1;
			if (search->opt_out_entity_ids != nullptr) {
				search->opt_out_entity_ids[count] = entity.id;

				if (new_count >= search->max_results) {
					break;
				}
			}

			count = new_count;
		}
	}

	return count;
}

odWorld::odWorld() = default;
odWorld::odWorld(odWorld&& other) = default;
odWorld& odWorld::operator=(odWorld&& other) = default;
odWorld::~odWorld() = default;

odChunkIterator::odChunkIterator(const odBounds& bounds)
	: x1{odChunkCoord_init(bounds.x)}, y1{odChunkCoord_init(bounds.y)},
	  x2{odChunkCoord_init(bounds.x + bounds.width)}, y2{odChunkCoord_init(bounds.y + bounds.width)} {
}

odChunkIterator odChunkIterator::begin() const {
	return *this;
}
odChunkIterator odChunkIterator::end() const {
	odChunkIterator chunk_iterator = *this;
	chunk_iterator.x1 = x2;
	chunk_iterator.y1 = y2;
	return chunk_iterator;
}
bool odChunkIterator::operator==(const odChunkIterator& other) const {
	return (
		(x1 == other.x1)
		&& (y1 == other.y1)
		&& (x2 == other.x2)
		&& (y2 == other.y2)
	);
}
bool odChunkIterator::operator!=(const odChunkIterator& other) const {
	return !(operator==(other));
}
odChunkIterator& odChunkIterator::operator++() {
	if (x1 < x2) {
		x1++;
	} else if (y1 < y2) {
		y1++;
	} else {
		OD_ERROR("Attempting to iterate chunk iterator past its end");
	}
	return *this;
}
odChunkIterator& odChunkIterator::operator++(int) {
	return operator++();
}
int32_t odChunkIterator::operator*() {
	return odChunkId_init_coords(x1, y1);
}

static int32_t odChunkIterator_contains_coords(const odChunkIterator* chunk_iterator, uint8_t x, uint8_t y) {
	if (!OD_DEBUG_CHECK(chunk_iterator != nullptr)) {
		return false;
	}

	if ((x < chunk_iterator->x1)
		|| (y < chunk_iterator->y1)
		|| (x > chunk_iterator->x2)
		|| (y > chunk_iterator->y2)
		|| (chunk_iterator->x1 >= chunk_iterator->x2)
		|| (chunk_iterator->y1 >= chunk_iterator->y2)) {
		return false;
	}

	return true;
}
static int32_t odChunkIterator_contains_chunk(const odChunkIterator* chunk_iterator, int32_t chunk_id) {
	return odChunkIterator_contains_coords(
		chunk_iterator,
		odChunkId_get_coord_x(chunk_id),
		odChunkId_get_coord_y(chunk_id));
}

static uint8_t odChunkCoord_init(float coord) {
	const int32_t coord_bitmask = (1 << OD_WORLD_CHUNK_COORD_MASK_BITS) - 1;
	return (static_cast<int32_t>(coord) >> OD_WORLD_CHUNK_COORD_DISCARD_BITS) & coord_bitmask;
}
// static int32_t odChunkId_init(float x, float y) {
// 	return (
// 		static_cast<int32_t>(odChunkCoord_init(x))
// 		+ static_cast<int32_t>((odChunkCoord_init(y) << OD_WORLD_CHUNK_COORD_MASK_BITS)));
// }
static int32_t odChunkId_init_coords(uint8_t x, uint8_t y) {
	return (
		static_cast<int32_t>(x)
		+ static_cast<int32_t>(y << OD_WORLD_CHUNK_COORD_MASK_BITS));
}
static uint8_t odChunkId_get_coord_x(int32_t chunk_id) {
	if (!OD_DEBUG_CHECK(chunk_id < 0)) {
		return 0;
	}

	const int32_t coord_bitmask = (1 << OD_WORLD_CHUNK_COORD_MASK_BITS) - 1;
	return static_cast<uint8_t>(chunk_id & coord_bitmask);
}
static uint8_t odChunkId_get_coord_y(int32_t chunk_id) {
	if (!OD_DEBUG_CHECK(chunk_id < 0)) {
		return 0;
	}

	const int32_t coord_bitmask = (1 << OD_WORLD_CHUNK_COORD_MASK_BITS) - 1;
	return static_cast<uint8_t>((chunk_id >> OD_WORLD_CHUNK_COORD_MASK_BITS) & coord_bitmask);
}

template struct odArrayT<odEntityStorage>;
