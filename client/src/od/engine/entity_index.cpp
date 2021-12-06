#include <od/engine/entity_index.hpp>

#include <cstdio>

#include <od/core/bounds.h>
#include <od/core/array.h>
#include <od/engine/tagset.h>
#include <od/engine/entity.hpp>

typedef uint8_t odEntityChunkCoord;

static_assert(
	OD_WORLD_CHUNK_COORD_MASK_BITS <= (8 * sizeof(odEntityChunkCoord)),
	"chunk coord mask must fit chunk coord type");

struct odChunkIterator {
	odEntityChunkCoord x_start;
	odEntityChunkCoord y_start;

	odEntityChunkCoord x1;
	odEntityChunkCoord y1;
	odEntityChunkCoord x2;
	odEntityChunkCoord y2;

	explicit odChunkIterator(const odBounds& bounds);

	odChunkIterator begin() const;
	odChunkIterator end() const;
	bool operator==(const odChunkIterator& other) const;
	bool operator!=(const odChunkIterator& other) const;
	odChunkIterator& operator++();
	odChunkIterator& operator++(int);
	/*chunk_id*/ int32_t operator*();
};

static bool odChunkIterator_contains_chunk_coords(const odChunkIterator* iter, odEntityChunkCoord x, odEntityChunkCoord y);
static bool odChunkIterator_contains_chunk_id(const odChunkIterator* iter, int32_t chunk_id);

static odEntityChunkCoord odChunkCoord_init(float coord);

// static int32_t odChunkId_init_coords(float x, float y);
static int32_t odChunkId_init_chunk_coords(odEntityChunkCoord x, odEntityChunkCoord y);
static odEntityChunkCoord odChunkId_get_coord_x(int32_t chunk_id);
static odEntityChunkCoord odChunkId_get_coord_y(int32_t chunk_id);

odChunkIterator::odChunkIterator(const odBounds& bounds)
: x_start{odChunkCoord_init(bounds.x)},
	y_start{odChunkCoord_init(bounds.y)},
	x1{x_start},
	y1{y_start},
	x2{odChunkCoord_init(bounds.x + bounds.width + (1 << OD_WORLD_CHUNK_COORD_DISCARD_BITS) - 1)},
	y2{odChunkCoord_init(bounds.y + bounds.height + (1 << OD_WORLD_CHUNK_COORD_DISCARD_BITS) - 1)} {
		// edge-case where width/height is big enough to exceed wrap-around, cover the full range
		const odEntityChunkCoord coord_bitmask = static_cast<odEntityChunkCoord>((1 << OD_WORLD_CHUNK_COORD_MASK_BITS) - 1);
		if (bounds.width >= (1 << OD_WORLD_CHUNK_COORD_BITS)) {
			x2 = static_cast<odEntityChunkCoord>((x1 - 1) & coord_bitmask);
		}
		if (bounds.height >= (1 << OD_WORLD_CHUNK_COORD_BITS)) {
			y2 = static_cast<odEntityChunkCoord>((y1 - 1) & coord_bitmask);
		}
}

odChunkIterator odChunkIterator::begin() const {
	return *this;
}
odChunkIterator odChunkIterator::end() const {
	odChunkIterator iter = *this;
	iter.x1 = x2;
	iter.y1 = y2;
	return iter;
}
bool odChunkIterator::operator==(const odChunkIterator& other) const {
	return (
		OD_DEBUG_CHECK(x_start == other.x_start)
		&& OD_DEBUG_CHECK(y_start == other.y_start)
		&& OD_DEBUG_CHECK(x2 == other.x2)
		&& OD_DEBUG_CHECK(y2 == other.y2)
		&& (x1 == other.x1)
		&& (y1 == other.y1)
	);
}
bool odChunkIterator::operator!=(const odChunkIterator& other) const {
	return !(operator==(other));
}
odChunkIterator& odChunkIterator::operator++() {
	const odEntityChunkCoord coord_bitmask = static_cast<odEntityChunkCoord>((1 << OD_WORLD_CHUNK_COORD_MASK_BITS) - 1);

	odEntityChunkCoord x_next = static_cast<odEntityChunkCoord>((x1 + 1) & coord_bitmask);
	odEntityChunkCoord y_next = static_cast<odEntityChunkCoord>((y1 + 1) & coord_bitmask);
	
	OD_TRACE(
		"this=%p, x1=%d, x2=%d, x_next=%d, y1=%d, y2=%d, y_next=%d",
		static_cast<const void*>(this),
		int(x1),
		int(x2),
		int(x_next),
		int(y1),
		int(y2),
		int(y_next));

	if (!OD_CHECK((x1 != x2) && (y1 != y2))) {
		OD_ERROR("Attempting to iterate chunk iterator past its end");
		return *this;
	}

	if ((x_next == x2) && (y_next == y2)) {
		x1 = x_next;
		y1 = y_next;
	}
	else if (x_next == x2) {
		y1 = y_next;
		x1 = x_start;
	} else {
		x1 = x_next;
	}

	return *this;
}
odChunkIterator& odChunkIterator::operator++(int) {
	return operator++();
}
int32_t odChunkIterator::operator*() {
	return odChunkId_init_chunk_coords(x1, y1);
}

static bool odChunkIterator_contains_chunk_coords(const odChunkIterator* iter, odEntityChunkCoord x, odEntityChunkCoord y) {
	if (!OD_DEBUG_CHECK(iter != nullptr)) {
		return false;
	}

	if (((iter->x1 < iter->x2) && ((x < iter->x1) || (x > iter->x2)))
		|| ((iter->y1 < iter->y2) && ((y < iter->y1) || (y > iter->y2)))
		// can wrap around on an axis; if so, reject coord if in continuous range of nonmatching values
		|| ((iter->x1 > iter->x2) && (x > iter->x1) && (x < iter->x2))
		|| ((iter->y1 > iter->y2) && (y > iter->y1) && (y < iter->y2))) {
		return false;
	}

	return true;
}
static bool odChunkIterator_contains_chunk_id(const odChunkIterator* iter, int32_t chunk_id) {
	return odChunkIterator_contains_chunk_coords(
		iter,
		odChunkId_get_coord_x(chunk_id),
		odChunkId_get_coord_y(chunk_id));
}
static odEntityChunkCoord odChunkCoord_init(float coord) {
	const int32_t coord_bitmask = static_cast<int32_t>((1 << OD_WORLD_CHUNK_COORD_MASK_BITS) - 1);
	return static_cast<odEntityChunkCoord>((static_cast<int32_t>(coord) >> OD_WORLD_CHUNK_COORD_DISCARD_BITS) & coord_bitmask);
}
// static int32_t odChunkId_init_coords(float x, float y) {
// 	return (
// 		static_cast<int32_t>(odChunkCoord_init(x))
// 		+ static_cast<int32_t>((odChunkCoord_init(y) << OD_WORLD_CHUNK_COORD_MASK_BITS)));
// }
static int32_t odChunkId_init_chunk_coords(odEntityChunkCoord x, odEntityChunkCoord y) {
	return (
		static_cast<int32_t>(x)
		+ static_cast<int32_t>(y << OD_WORLD_CHUNK_COORD_MASK_BITS));
}
static odEntityChunkCoord odChunkId_get_coord_x(int32_t chunk_id) {
	if (!OD_DEBUG_CHECK(chunk_id >= 0)) {
		return 0;
	}

	const int32_t coord_bitmask = (1 << OD_WORLD_CHUNK_COORD_MASK_BITS) - 1;
	return static_cast<odEntityChunkCoord>(chunk_id & coord_bitmask);
}
static odEntityChunkCoord odChunkId_get_coord_y(int32_t chunk_id) {
	if (!OD_DEBUG_CHECK(chunk_id >= 0)) {
		return 0;
	}

	const int32_t coord_bitmask = (1 << OD_WORLD_CHUNK_COORD_MASK_BITS) - 1;
	return static_cast<odEntityChunkCoord>((chunk_id >> OD_WORLD_CHUNK_COORD_MASK_BITS) & coord_bitmask);
}

static const char* odEntityIndex_chunk_get_debug_string(const odArrayT<odEntity>* world_chunk) {
	if (world_chunk == nullptr) {
		return "null";
	}

	int32_t entities_count = odArray_get_count(world_chunk);
	const void* entities = odArray_begin_const(world_chunk);
	const char* entities_str = nullptr;

	if ((entities != nullptr) && (entities_count <= 64)) {
		entities_str = odDebugString_format_array(
			reinterpret_cast<const char*(*)(const void*)>(&odEntity_get_debug_string),
			entities,
			entities_count,
			sizeof(odEntity));
	}
	if (entities_str == nullptr) {
		entities_str = "\"...\"";
	}

	return odDebugString_format(
		"{\"count\": %d, \"entities\": [%s]}",
		entities_count,
		entities_str);
}
const char* odEntityIndex_get_debug_string(const odEntityIndex* entity_index) {
	if (entity_index == nullptr) {
		return "null";
	}

	int32_t entities_count = odArray_get_count(&entity_index->entities);
	const void* entities = odArray_begin_const(&entity_index->entities);
	const char* entities_str = nullptr;

	if ((entities != nullptr) && (entities_count <= 64)) {
		entities_str = odDebugString_format_array(
			reinterpret_cast<const char*(*)(const void*)>(&odEntity_get_debug_string),
			odArray_begin_const(&entity_index->entities),
			odArray_get_count(&entity_index->entities),
			sizeof(odEntity));
	}

	const char* chunks_str = odDebugString_format_array(
		reinterpret_cast<const char*(*)(const void*)>(&odEntityIndex_chunk_get_debug_string),
		&entity_index->chunks,
		OD_WORLD_CHUNK_ID_MAX,
		sizeof(odArrayT<odEntity>));
	if (chunks_str == nullptr) {
		chunks_str = "\"...\"";
	}

	return odDebugString_format(
		"{\"count\": %d, \"entities\": [%s], \"chunks\": [%s]}",
		entities_count,
		entities_str,
		chunks_str);
}
bool odEntityIndex_init(odEntityIndex* entity_index) {
	OD_DEBUG("entity_index=%p", static_cast<const void*>(entity_index));

	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return false;
	}

	odEntityIndex_destroy(entity_index);

	return true;
}
void odEntityIndex_destroy(odEntityIndex* entity_index) {
	OD_DEBUG("entity_index=%p", static_cast<const void*>(entity_index));

	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return;
	}

	odArray_destroy(&entity_index->entities);

	for (int32_t i = 0; i < OD_WORLD_CHUNK_ID_MAX; i++) {
		if (!OD_CHECK(entity_index->chunks[i].set_capacity(0))) {
			continue;
		}
	}
}
static OD_NO_DISCARD bool odEntityIndex_chunk_find_collider(odEntityIndex* entity_index, int32_t chunk_id, int32_t entity_id, int32_t* opt_out_chunk_index) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((chunk_id >= 0) && (chunk_id < OD_WORLD_CHUNK_ID_MAX))
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < odArray_get_count(&entity_index->entities)))) {
		return false;
	}

	odFastArrayT<odEntityCollider>* chunk_collider_array = &entity_index->chunks[chunk_id];
	const odEntityCollider* chunk_colliders = entity_index->chunks[chunk_id].begin();
	if (!OD_DEBUG_CHECK((chunk_colliders != nullptr) || (chunk_collider_array->count == 0))) {
		return false;
	}

	int32_t chunk_index = 0;
	bool found = false;
	for (int32_t i = 0; i < chunk_collider_array->count; i++) {
		if (chunk_colliders[i].id == entity_id) {
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
static OD_NO_DISCARD bool odEntityIndex_chunk_unset_collider(odEntityIndex* entity_index, int32_t chunk_id, int32_t entity_id) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((chunk_id >= 0) && (chunk_id < OD_WORLD_CHUNK_ID_MAX))
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < odArray_get_count(&entity_index->entities)))) {
		return false;
	}

	odFastArrayT<odEntityCollider>* chunk_collider_array = &entity_index->chunks[chunk_id];
	int32_t chunk_index = 0;
	if (!OD_CHECK(odEntityIndex_chunk_find_collider(entity_index, chunk_id, entity_id, &chunk_index))) {
		return false;
	}

	if (!OD_DEBUG_CHECK((chunk_index >= 0) && (chunk_index < chunk_collider_array->count))) {
		return false;
	}

	if (!OD_CHECK(chunk_collider_array->swap_pop(chunk_index))) {
		return false;
	}

	return true;
}
static OD_NO_DISCARD bool odEntityIndex_chunk_set_collider(odEntityIndex* entity_index, int32_t chunk_id, const odEntityCollider* collider) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((chunk_id >= 0) && (chunk_id < OD_WORLD_CHUNK_ID_MAX))
		|| !OD_DEBUG_CHECK(odEntityCollider_check_valid(collider))
		|| !OD_DEBUG_CHECK(collider->id < odArray_get_count(&entity_index->entities))) {
		return false;
	}

	odFastArrayT<odEntityCollider>* chunk_collider_array = &entity_index->chunks[chunk_id];
	int32_t chunk_index = 0;
	if (!odEntityIndex_chunk_find_collider(entity_index, chunk_id, collider->id, &chunk_index)) {
		if (!OD_CHECK(chunk_collider_array->extend(collider, 1))) {
			return false;
		}

		return true;
	}

	if (!OD_DEBUG_CHECK((chunk_index >= 0) && (chunk_index < chunk_collider_array->count))) {
		return false;
	}

	(*chunk_collider_array)[chunk_index] = *collider;

	return true;
}
const odEntity* odEntityIndex_get(const odEntityIndex* entity_index, int32_t entity_id) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(entity_id >= 0)) {
		return nullptr;
	}

	if (entity_id >= odArray_get_count(&entity_index->entities)) {
		return nullptr;
	}

	const odEntity* entity = entity_index->entities[entity_id];
	if (!OD_DEBUG_CHECK(odEntity_check_valid(entity))) {
		return nullptr;
	}

	return entity;
}
bool odEntityIndex_set_collider(odEntityIndex* entity_index, const odEntityCollider* collider) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(odEntityCollider_check_valid(collider))) {
		return false;
	}

	if (!OD_CHECK(odArray_ensure_count(&entity_index->entities, collider->id + 1))) {
		return false;
	}

	odEntity* old_entity = entity_index->entities[collider->id];
	if (!OD_DEBUG_CHECK(odEntity_check_valid(old_entity))) {
		return false;
	}

	odChunkIterator old_bounds_chunks{old_entity->collider.bounds};
	odChunkIterator new_bounds_chunks{collider->bounds};
	for (int32_t chunk_id: old_bounds_chunks) {
		if (!odChunkIterator_contains_chunk_id(&new_bounds_chunks, chunk_id)) {
			if (!OD_CHECK(odEntityIndex_chunk_unset_collider(entity_index, chunk_id, collider->id))) {
				return false;
			}
		}
	}

	for (int32_t chunk_id: new_bounds_chunks) {
		if (!OD_CHECK(odEntityIndex_chunk_set_collider(entity_index, chunk_id, collider))) {
			return false;
		}
	}

	old_entity->collider = *collider;

	return true;
}
bool odEntityIndex_set_sprite(odEntityIndex* entity_index, int32_t entity_id, const odEntitySprite* sprite) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < odArray_get_count(&entity_index->entities)))
		|| !OD_DEBUG_CHECK(odEntitySprite_check_valid(sprite))) {
		return false;
	}

	if (!OD_CHECK(odArray_ensure_count(&entity_index->entities, entity_id + 1))) {
		return false;
	}

	odEntity* old_entity = entity_index->entities[entity_id];
	if (!OD_DEBUG_CHECK(old_entity != nullptr)
		|| !OD_DEBUG_CHECK(odEntity_check_valid(old_entity))) {
		return false;
	}

	old_entity->sprite = *sprite;

	return true;
}
bool odEntityIndex_set(odEntityIndex* entity_index, const odEntity* entity) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(odEntity_check_valid(entity))) {
		return false;
	}

	if (!OD_CHECK(odEntityIndex_set_collider(entity_index, &entity->collider))) {
		return false;
	}

	if (!OD_CHECK(odEntityIndex_set_sprite(entity_index, entity->collider.id, &entity->sprite))) {
		return false;
	}

	return true;
}
int32_t odEntityIndex_search(const odEntityIndex* entity_index, const odEntitySearch* search) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(odEntitySearch_check_valid(search))) {
		return 0;
	}

	int32_t count = 0;
	odChunkIterator search_bounds{search->bounds};
	for (int32_t chunk_id: search_bounds) {
		for (const odEntityCollider& collider: entity_index->chunks[chunk_id]) {
			if (count >= search->max_entity_ids) {
				break;
			}

			if (!OD_DEBUG_CHECK(odEntityCollider_check_valid(&collider))) {
				break;
			}

			if (!odEntitySearch_matches_collider(search, &collider)) {
				continue;
			}

			bool seen_before = false;
			for (int32_t i = 0; i < count; i++) {
				if (search->out_entity_ids[i] == collider.id) {
					seen_before = true;
					break;
				}
			}
			if (seen_before) {
				continue;
			}

			search->out_entity_ids[count] = collider.id;
			count++;
		}
	}

	return count;
}

odEntityIndex::odEntityIndex() = default;
odEntityIndex::odEntityIndex(odEntityIndex&& other) = default;
odEntityIndex& odEntityIndex::operator=(odEntityIndex&& other) = default;
odEntityIndex::~odEntityIndex() = default;

const char* odEntitySearch_get_debug_string(const odEntitySearch* search) {
	if (search == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"out_entity_ids\": \"%p\", \"max_entity_ids\": %d, \"tagset\": %s, \"bounds\": %s}",
		static_cast<const void*>(search->out_entity_ids),
		search->max_entity_ids,
		odTagset_get_debug_string(&search->tagset),
		odBounds_get_debug_string(&search->bounds));
}
bool odEntitySearch_check_valid(const odEntitySearch* search) {
	if (!OD_CHECK(search != nullptr)) {
		return false;
	}

	if (!OD_CHECK(search->out_entity_ids != nullptr)
		|| !OD_CHECK(search->max_entity_ids >= 0)
		|| !OD_CHECK(odBounds_check_valid(&search->bounds))) {
		return false;
	}

	return true;
}
bool odEntitySearch_matches_collider(const odEntitySearch* search, const odEntityCollider* collider) {
	if (!OD_DEBUG_CHECK(odEntitySearch_check_valid(search))
		|| !OD_DEBUG_CHECK(odEntityCollider_check_valid(collider))) {
		return false;
	}

	if (!odBounds_collides(&collider->bounds, &search->bounds)
		|| !odTagset_intersects(&collider->tagset, &search->tagset)) {
		return false;
	}

	return true;
}
