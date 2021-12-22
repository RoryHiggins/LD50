#include <od/engine/entity_index.hpp>

#include <cmath>
#include <cstdio>

#include <od/core/bounds.h>
#include <od/core/array.hpp>
#include <od/core/vertex.h>
#include <od/platform/primitive.h>
#include <od/engine/tagset.h>
#include <od/engine/entity.hpp>

typedef uint8_t odEntityChunkCoord;
typedef int32_t odEntityChunkId;

struct odEntityStorage {
	odEntity entity;
};
struct odEntityChunkIterator {
	odEntityChunkCoord x_start;
	odEntityChunkCoord y_start;

	odEntityChunkCoord x1;
	odEntityChunkCoord y1;
	odEntityChunkCoord x2;
	odEntityChunkCoord y2;

	explicit odEntityChunkIterator(const odBounds& bounds);

	odEntityChunkIterator begin() const;
	odEntityChunkIterator end() const;
	bool operator==(const odEntityChunkIterator& other) const;
	bool operator!=(const odEntityChunkIterator& other) const;
	odEntityChunkIterator& operator++();
	odEntityChunkIterator& operator++(int);
	odEntityChunkId operator*();
};

extern template struct odTrivialArrayT<odEntityStorage>;

OD_NO_DISCARD bool
odEntityStorage_check_valid(const odEntityStorage* storage);

static OD_NO_DISCARD bool
odEntityChunkIterator_contains_chunk_coords(const odEntityChunkIterator* iter, odEntityChunkCoord x, odEntityChunkCoord y);
static OD_NO_DISCARD bool
odEntityChunkIterator_contains_chunk_id(const odEntityChunkIterator* iter, odEntityChunkId chunk_id);

static OD_NO_DISCARD odEntityChunkCoord
odChunkCoord_init(float coord);

static OD_NO_DISCARD odEntityChunkId
odEntityChunkId_init_chunk_coords(odEntityChunkCoord x, odEntityChunkCoord y);
static OD_NO_DISCARD odEntityChunkCoord
odEntityChunkId_get_coord_x(odEntityChunkId chunk_id);
static OD_NO_DISCARD odEntityChunkCoord
odEntityChunkId_get_coord_y(odEntityChunkId chunk_id);

static OD_NO_DISCARD const char*
odEntityIndex_chunk_get_debug_string(const odEntityChunk* chunk);
static OD_NO_DISCARD bool
odEntityIndex_chunk_find_collider(odEntityIndex* entity_index, odEntityChunkId chunk_id, odEntityId entity_id, int32_t* opt_out_chunk_index);
static OD_NO_DISCARD bool
odEntityIndex_chunk_unset_collider(odEntityIndex* entity_index, odEntityChunkId chunk_id, odEntityId entity_id);
static OD_NO_DISCARD bool
odEntityIndex_chunk_set_collider(odEntityIndex* entity_index, odEntityChunkId chunk_id, const odEntityCollider* collider);
static OD_NO_DISCARD bool
odEntityIndex_ensure_count(odEntityIndex* entity_index, int32_t min_count);
static OD_NO_DISCARD odEntityStorage*
odEntityIndex_get_or_allocate_storage(odEntityIndex* entity_index, odEntityId entity_id);
static OD_NO_DISCARD bool
odEntityIndex_update_vertices_impl(odEntityIndex* entity_index, const odEntityStorage* storage);
static OD_NO_DISCARD bool
odEntityIndex_set_collider_impl(odEntityIndex* entity_index, odEntityCollider* old_collider, const odEntityCollider* collider);
static OD_NO_DISCARD bool
odEntityIndex_set_sprite_impl(odEntityIndex* entity_index, odEntitySprite* old_sprite, const odEntitySprite* sprite);

bool odEntityStorage_check_valid(const odEntityStorage* storage) {
	if (!OD_CHECK(storage != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odEntity_check_valid(&storage->entity))) {
		return false;
	}

	return true;
}

odEntityChunkIterator odEntityChunkIterator::begin() const {
	return *this;
}
odEntityChunkIterator odEntityChunkIterator::end() const {
	odEntityChunkIterator iter = *this;
	iter.x1 = x2;
	iter.y1 = y2;
	return iter;
}
bool odEntityChunkIterator::operator==(const odEntityChunkIterator& other) const {
	return (
		OD_DEBUG_CHECK(x_start == other.x_start)
		&& OD_DEBUG_CHECK(y_start == other.y_start)
		&& OD_DEBUG_CHECK(x2 == other.x2)
		&& OD_DEBUG_CHECK(y2 == other.y2)
		&& (x1 == other.x1)
		&& (y1 == other.y1)
	);
}
bool odEntityChunkIterator::operator!=(const odEntityChunkIterator& other) const {
	return !(operator==(other));
}
odEntityChunkIterator& odEntityChunkIterator::operator++() {
	const odEntityChunkCoord coord_bitmask = static_cast<odEntityChunkCoord>((1 << OD_ENTITY_CHUNK_COORD_MASK_BITS) - 1);

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

	if (!OD_DEBUG_CHECK((x1 != x2) && (y1 != y2))) {
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
odEntityChunkIterator& odEntityChunkIterator::operator++(int) {
	return operator++();
}
odEntityChunkId odEntityChunkIterator::operator*() {
	return odEntityChunkId_init_chunk_coords(x1, y1);
}

bool odEntityChunkIterator_contains_chunk_coords(const odEntityChunkIterator* iter, odEntityChunkCoord x, odEntityChunkCoord y) {
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
bool odEntityChunkIterator_contains_chunk_id(const odEntityChunkIterator* iter, odEntityChunkId chunk_id) {
	return odEntityChunkIterator_contains_chunk_coords(
		iter,
		odEntityChunkId_get_coord_x(chunk_id),
		odEntityChunkId_get_coord_y(chunk_id));
}
odEntityChunkIterator::odEntityChunkIterator(const odBounds& bounds)
: x_start{odChunkCoord_init(bounds.x1)},
	y_start{odChunkCoord_init(bounds.y1)},
	x1{x_start},
	y1{y_start},
	x2{odChunkCoord_init(bounds.x2 + (1 << OD_ENTITY_CHUNK_COORD_DISCARD_BITS) - 1)},
	y2{odChunkCoord_init(bounds.y2 + (1 << OD_ENTITY_CHUNK_COORD_DISCARD_BITS) - 1)} {
		// in edge-case where width/height is big enough to exceed wrap-around, cover the full range
		const odEntityChunkCoord coord_bitmask =
			static_cast<odEntityChunkCoord>((1 << OD_ENTITY_CHUNK_COORD_MASK_BITS) - 1);
		if ((bounds.x2 - bounds.x1) >= (1 << OD_ENTITY_CHUNK_COORD_BITS)) {
			x2 = static_cast<odEntityChunkCoord>((x1 - 1) & coord_bitmask);
		}
		if ((bounds.y2 - bounds.y1) >= (1 << OD_ENTITY_CHUNK_COORD_BITS)) {
			y2 = static_cast<odEntityChunkCoord>((y1 - 1) & coord_bitmask);
		}
}

odEntityChunkCoord odChunkCoord_init(float coord) {
	const int32_t coord_bitmask =
		static_cast<int32_t>((1 << OD_ENTITY_CHUNK_COORD_MASK_BITS) - 1);
	return static_cast<odEntityChunkCoord>(
		(static_cast<int32_t>(coord) >> OD_ENTITY_CHUNK_COORD_DISCARD_BITS) & coord_bitmask
	);
}
odEntityChunkId odEntityChunkId_init_chunk_coords(odEntityChunkCoord x, odEntityChunkCoord y) {
	return (
		static_cast<odEntityChunkId>(x)
		+ static_cast<odEntityChunkId>(y << OD_ENTITY_CHUNK_COORD_MASK_BITS));
}
odEntityChunkCoord odEntityChunkId_get_coord_x(odEntityChunkId chunk_id) {
	if (!OD_DEBUG_CHECK(chunk_id >= 0)) {
		return 0;
	}

	const int32_t coord_bitmask = (1 << OD_ENTITY_CHUNK_COORD_MASK_BITS) - 1;
	return static_cast<odEntityChunkCoord>(chunk_id & coord_bitmask);
}
odEntityChunkCoord odEntityChunkId_get_coord_y(odEntityChunkId chunk_id) {
	if (!OD_DEBUG_CHECK(chunk_id >= 0)) {
		return 0;
	}

	const int32_t coord_bitmask = (1 << OD_ENTITY_CHUNK_COORD_MASK_BITS) - 1;
	return static_cast<odEntityChunkCoord>((chunk_id >> OD_ENTITY_CHUNK_COORD_MASK_BITS) & coord_bitmask);
}

const char* odEntityIndex_chunk_get_debug_string(const odEntityChunk* chunk) {
	if (chunk == nullptr) {
		return "null";
	}

	const char* entities_str = nullptr;
	if (chunk->colliders.count <= 64) {
		entities_str = odDebugString_format_array(
			reinterpret_cast<const char*(*)(const void*)>(&odEntity_get_debug_string),
			chunk->colliders.begin(),
			chunk->colliders.count,
			sizeof(odEntityStorage));
	}
	if (entities_str == nullptr) {
		entities_str = "\"...\"";
	}

	return odDebugString_format(
		"{\"count\": %d, \"entities\": [%s]}",
		chunk->colliders.count,
		entities_str);
}
bool odEntityIndex_chunk_find_collider(odEntityIndex* entity_index, odEntityChunkId chunk_id, odEntityId entity_id, int32_t* opt_out_chunk_index) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((chunk_id >= 0) && (chunk_id < OD_ENTITY_CHUNK_ID_COUNT))
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < entity_index->entities.count))) {
		return false;
	}

	odEntityChunk& chunk = entity_index->chunks[chunk_id];
	int32_t chunk_index = 0;
	bool found = false;
	for (int32_t i = 0; i < chunk.colliders.count; i++) {
		if (chunk.colliders[i].id == entity_id) {
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
bool odEntityIndex_chunk_unset_collider(odEntityIndex* entity_index, odEntityChunkId chunk_id, odEntityId entity_id) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((chunk_id >= 0) && (chunk_id < OD_ENTITY_CHUNK_ID_COUNT))
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < entity_index->entities.count))) {
		return false;
	}

	odEntityChunk& chunk = entity_index->chunks[chunk_id];
	int32_t chunk_index = 0;
	if (!OD_CHECK(odEntityIndex_chunk_find_collider(entity_index, chunk_id, entity_id, &chunk_index))) {
		return false;
	}

	if (!OD_DEBUG_CHECK((chunk_index >= 0) && (chunk_index < chunk.colliders.count))) {
		return false;
	}

	if (!OD_CHECK(chunk.colliders.swap_pop(chunk_index))) {
		return false;
	}

	return true;
}
bool odEntityIndex_chunk_set_collider(odEntityIndex* entity_index, odEntityChunkId chunk_id, const odEntityCollider* collider) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((chunk_id >= 0) && (chunk_id < OD_ENTITY_CHUNK_ID_COUNT))
		|| !OD_DEBUG_CHECK(odEntityCollider_check_valid(collider))
		|| !OD_DEBUG_CHECK(odBounds_is_collidable(&collider->bounds))
		|| !OD_DEBUG_CHECK(collider->id < entity_index->entities.count)) {
		return false;
	}

	odEntityChunk& chunk = entity_index->chunks[chunk_id];
	int32_t chunk_index = 0;
	if (!odEntityIndex_chunk_find_collider(entity_index, chunk_id, collider->id, &chunk_index)) {
		if (!OD_CHECK(chunk.colliders.extend(collider, 1))) {
			return false;
		}

		return true;
	}

	if (!OD_DEBUG_CHECK((chunk_index >= 0) && (chunk_index < chunk.colliders.count))) {
		return false;
	}

	chunk.colliders[chunk_index] = *collider;

	return true;
}
bool odEntityIndex_ensure_count(odEntityIndex* entity_index, int32_t min_count) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(min_count >= 0)) {
		return false;
	}

	if (entity_index->entities.count < min_count) {
		if (!OD_CHECK(entity_index->entities.set_count(min_count))) {
			return false;
		}
	}

	int32_t min_vertices_count = min_count * 6;
	if (entity_index->entity_vertices.count < min_vertices_count) {
		if (!OD_CHECK(entity_index->entity_vertices.set_count(min_vertices_count))) {
			return false;
		}
	}

	return true;
}
odEntityStorage* odEntityIndex_get_or_allocate_storage(odEntityIndex* entity_index, odEntityId entity_id) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(entity_id >= 0)) {
		return nullptr;
	}

	if (!OD_CHECK(odEntityIndex_ensure_count(entity_index, entity_id + 1))) {
		return nullptr;
	}

	odEntityStorage* old_entity = entity_index->entities.get(entity_id);
	if (!OD_DEBUG_CHECK(odEntityStorage_check_valid(old_entity))) {
		return nullptr;
	}

	return old_entity;
}
bool odEntityIndex_update_vertices_impl(odEntityIndex* entity_index, const odEntityStorage* storage) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(odEntityStorage_check_valid(storage))) {
		return false;
	}

	odRectPrimitive rect{};
	odEntity_get_rect(&storage->entity, &rect);

	int32_t vertex_index = static_cast<int32_t>(storage->entity.collider.id) * OD_RECT_PRIMITIVE_VERTEX_COUNT;

	if (!OD_DEBUG_CHECK((vertex_index + OD_RECT_PRIMITIVE_VERTEX_COUNT) <= entity_index->entity_vertices.get_count())) {
		return false;
	}

	odVertex* vertices = entity_index->entity_vertices.get(vertex_index);
	if (!OD_DEBUG_CHECK(odVertex_check_valid_batch(vertices, OD_RECT_PRIMITIVE_VERTEX_COUNT))) {
		return false;
	}

	odRectPrimitive_get_vertices(&rect, vertices);

	for (int32_t i = 0; i < OD_ENTITY_VERTEX_COUNT; i++) {
		odVertex_transform(vertices + i, &storage->entity.sprite.transform);
	}

	return true;
}
bool odEntityIndex_set_collider_impl(odEntityIndex* entity_index, odEntityCollider* old_collider, const odEntityCollider* collider) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(odEntityCollider_check_valid(old_collider))
		|| !OD_DEBUG_CHECK(odEntityCollider_check_valid(collider))) {
		return false;
	}

	odEntityChunkIterator old_bounds_chunks{old_collider->bounds};
	odEntityChunkIterator new_bounds_chunks{collider->bounds};
	for (odEntityChunkId chunk_id: old_bounds_chunks) {
		if (!odEntityChunkIterator_contains_chunk_id(&new_bounds_chunks, chunk_id)) {
			if (!OD_CHECK(odEntityIndex_chunk_unset_collider(entity_index, chunk_id, collider->id))) {
				return false;
			}
		}
	}

	for (odEntityChunkId chunk_id: new_bounds_chunks) {
		if (!OD_CHECK(odEntityIndex_chunk_set_collider(entity_index, chunk_id, collider))) {
			return false;
		}
	}

	*old_collider = *collider;

	return true;
}
bool odEntityIndex_set_sprite_impl(odEntityIndex* entity_index, odEntitySprite* old_sprite, const odEntitySprite* sprite) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(odEntitySprite_check_valid(old_sprite))
		|| !OD_DEBUG_CHECK(odEntitySprite_check_valid(sprite))) {
		return false;
	}

	*old_sprite = *sprite;

	OD_MAYBE_UNUSED(entity_index);

	return true;
}
const char* odEntityIndex_get_debug_string(const odEntityIndex* entity_index) {
	if (entity_index == nullptr) {
		return "null";
	}

	const char* entities_str = nullptr;

	if ((entity_index->entities.begin() != nullptr) && (entity_index->entities.count <= 64)) {
		entities_str = odDebugString_format_array(
			reinterpret_cast<const char*(*)(const void*)>(&odEntity_get_debug_string),
			entity_index->entities.begin(),
			entity_index->entities.count,
			sizeof(odEntityStorage));
	}

	const char* chunks_str = odDebugString_format_array(
		reinterpret_cast<const char*(*)(const void*)>(&odEntityIndex_chunk_get_debug_string),
		&entity_index->chunks,
		OD_ENTITY_CHUNK_ID_COUNT,
		sizeof(odEntityChunk));
	if (chunks_str == nullptr) {
		chunks_str = "\"...\"";
	}

	return odDebugString_format(
		"{\"count\": %d, \"entities\": [%s], \"chunks\": [%s]}",
		entity_index->entities.count,
		entities_str,
		chunks_str);
}
bool odEntityIndex_init(odEntityIndex* entity_index) {
	OD_DEBUG("entity_index=%p", static_cast<const void*>(entity_index));

	if (!OD_CHECK(entity_index != nullptr)) {
		return false;
	}

	odEntityIndex_destroy(entity_index);

	return true;
}
void odEntityIndex_destroy(odEntityIndex* entity_index) {
	OD_DEBUG("entity_index=%p", static_cast<const void*>(entity_index));

	if (!OD_CHECK(entity_index != nullptr)) {
		return;
	}

	if (!OD_CHECK(entity_index->entities.set_count(0))) {
		return;
	}

	if (!OD_CHECK(entity_index->entity_vertices.set_count(0))) {
		return;
	}

	for (odEntityChunkId i = 0; i < OD_ENTITY_CHUNK_ID_COUNT; i++) {
		if (!OD_CHECK(entity_index->chunks[i].colliders.set_capacity(0))) {
			return;
		}
	}
}
odEntityId odEntityIndex_get_count(const odEntityIndex* entity_index) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return false;
	}

	return entity_index->entities.count;
}
const struct odVertex* odEntityIndex_get_vertices(const odEntityIndex* entity_index, odEntityId entity_id) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < entity_index->entities.count))) {
		return nullptr;
	}

	int32_t vertex_index = static_cast<int32_t>(entity_id) * 6;
	if (!OD_DEBUG_CHECK(vertex_index < entity_index->entity_vertices.count)) {
		return nullptr;
	}

	const odVertex* vertices = entity_index->entity_vertices.get(vertex_index);
	if (!OD_DEBUG_CHECK(odVertex_check_valid_batch(vertices, OD_RECT_PRIMITIVE_VERTEX_COUNT))) {
		return nullptr;
	}

	return vertices;
}
const struct odVertex* odEntityIndex_get_all_vertices(const odEntityIndex* entity_index, int32_t* out_vertex_count) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(out_vertex_count != nullptr)) {
		return nullptr;
	}

	*out_vertex_count = 0;

	if (entity_index->entity_vertices.count == 0) {
		return nullptr;
	}

	const odVertex* vertices = entity_index->entity_vertices.begin();
	if (!OD_CHECK(vertices != nullptr)) {
		return nullptr;
	}

	*out_vertex_count = entity_index->entity_vertices.count;
	return vertices;
}
const odEntity* odEntityIndex_get(const odEntityIndex* entity_index, odEntityId entity_id) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < entity_index->entities.count))) {
		return nullptr;
	}

	return &entity_index->entities[entity_id].entity;
}
void odEntityIndex_set_collider(odEntityIndex* entity_index, const odEntityCollider* collider) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(odEntityCollider_check_valid(collider))) {
		return;
	}

	odEntityStorage* old_entity_storage = odEntityIndex_get_or_allocate_storage(entity_index, collider->id);
	if (!OD_DEBUG_CHECK(odEntityStorage_check_valid(old_entity_storage))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_set_collider_impl(entity_index, &old_entity_storage->entity.collider, collider))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_update_vertices_impl(entity_index, old_entity_storage))) {
		return;
	}
}
void odEntityIndex_set_sprite(odEntityIndex* entity_index, odEntityId entity_id, const odEntitySprite* sprite) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < entity_index->entities.count))
		|| !OD_DEBUG_CHECK(odEntitySprite_check_valid(sprite))) {
		return;
	}

	odEntityStorage* old_entity_storage = odEntityIndex_get_or_allocate_storage(entity_index, entity_id);
	if (!OD_DEBUG_CHECK(odEntityStorage_check_valid(old_entity_storage))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_set_sprite_impl(entity_index, &old_entity_storage->entity.sprite, sprite))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_update_vertices_impl(entity_index, old_entity_storage))) {
		return;
	}
}
void odEntityIndex_set(odEntityIndex* entity_index, const odEntity* entity) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(odEntity_check_valid(entity))) {
		return;
	}

	odEntityStorage* old_entity_storage = odEntityIndex_get_or_allocate_storage(entity_index, entity->collider.id);
	if (!OD_DEBUG_CHECK(odEntityStorage_check_valid(old_entity_storage))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_set_collider_impl(entity_index, &old_entity_storage->entity.collider, &entity->collider))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_set_sprite_impl(entity_index, &old_entity_storage->entity.sprite, &entity->sprite))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_update_vertices_impl(entity_index, old_entity_storage))) {
		return;
	}
}
/*num_results*/ int32_t odEntityIndex_search(const odEntityIndex* entity_index, const odEntitySearch* search) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(odEntitySearch_check_valid(search))) {
		return 0;
	}

	int32_t count = 0;
	odEntityChunkIterator search_bounds{search->bounds};
	for (odEntityChunkId chunk_id: search_bounds) {
		for (const odEntityCollider& collider: entity_index->chunks[chunk_id].colliders) {
			if (count >= search->max_results) {
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
				if (search->out_results[i] == collider.id) {
					seen_before = true;
					break;
				}
			}
			if (seen_before) {
				continue;
			}

			search->out_results[count] = collider.id;
			count++;
		}
	}

	return count;
}
odEntityIndex::odEntityIndex()
: entities{}, chunks{} {
}
odEntityIndex::odEntityIndex(odEntityIndex&& other) = default;
odEntityIndex& odEntityIndex::operator=(odEntityIndex&& other) = default;
odEntityIndex::~odEntityIndex() = default;

const char* odEntitySearch_get_debug_string(const odEntitySearch* search) {
	if (search == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"out_results\": \"%p\", \"max_results\": %d, \"tagset\": %s, \"bounds\": %s}",
		static_cast<const void*>(search->out_results),
		search->max_results,
		odTagset_get_debug_string(&search->tagset),
		odBounds_get_debug_string(&search->bounds));
}
bool odEntitySearch_check_valid(const odEntitySearch* search) {
	if (!OD_CHECK(search != nullptr)) {
		return false;
	}

	if (!OD_CHECK(search->out_results != nullptr)
		|| !OD_CHECK(search->max_results >= 0)
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

template struct odTrivialArrayT<odEntityStorage>;

static_assert(
	OD_ENTITY_CHUNK_COORD_MASK_BITS <= (8 * sizeof(odEntityChunkCoord)),
	"chunk coord must fit chunk coord type");
static_assert(
	OD_ENTITY_CHUNK_ID_BITS <= 14,
	"chunk count must not exceed 2^14 chunks (this limit is already excessive)");
