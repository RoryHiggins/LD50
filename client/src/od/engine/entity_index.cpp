#include <od/engine/entity_index.hpp>

#include <cmath>
#include <cstdio>

#include <od/core/math.h>
#include <od/core/bounds.h>
#include <od/core/array.hpp>
#include <od/core/vertex.h>
#include <od/platform/primitive.h>
#include <od/engine/tagset.h>
#include <od/engine/entity.hpp>

typedef uint8_t odEntityChunkCoord;
typedef int32_t odEntityChunkId;

struct odEntityIndexEntity {
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

OD_NO_DISCARD bool
odEntityIndexEntity_check_valid(const odEntityIndexEntity* entity);

static OD_NO_DISCARD bool
odEntityChunkIterator_contains_chunk_coords(const odEntityChunkIterator* iter, odEntityChunkCoord x, odEntityChunkCoord y);
static OD_NO_DISCARD bool
odEntityChunkIterator_contains_chunk_id(const odEntityChunkIterator* iter, odEntityChunkId chunk_id);

static OD_NO_DISCARD odEntityChunkCoord
odChunkCoord_init(float value);
static OD_NO_DISCARD float
odChunkCoord_get_value(odEntityChunkCoord coord);

static OD_NO_DISCARD odEntityChunkId
odEntityChunkId_init_chunk_coords(odEntityChunkCoord x, odEntityChunkCoord y);
static OD_NO_DISCARD odEntityChunkCoord
odEntityChunkId_get_coord_x(odEntityChunkId chunk_id);
static OD_NO_DISCARD odEntityChunkCoord
odEntityChunkId_get_coord_y(odEntityChunkId chunk_id);

static OD_NO_DISCARD const char*
odEntityIndex_chunk_get_debug_string(const odEntityChunk* chunk);
static OD_NO_DISCARD bool
odEntityIndex_chunk_find_collider(odEntityIndex* entity_index, odEntityChunkId chunk_id, odEntityId entity_id, int32_t* out_chunk_index);
static OD_NO_DISCARD bool
odEntityIndex_chunk_unset_collider(odEntityIndex* entity_index, odEntityChunkId chunk_id, odEntityId entity_id);
static OD_NO_DISCARD bool
odEntityIndex_chunk_set_collider(odEntityIndex* entity_index, odEntityChunkId chunk_id, const odEntityCollider* collider);
static OD_NO_DISCARD bool
odEntityIndex_ensure_count(odEntityIndex* entity_index, int32_t min_count);
static OD_NO_DISCARD odEntityIndexEntity*
odEntityIndex_get_or_add_allocation(odEntityIndex* entity_index, odEntityId entity_id);
static OD_NO_DISCARD bool
odEntityIndex_update_vertices_impl(odEntityIndex* entity_index, const odEntityIndexEntity* entity);
static OD_NO_DISCARD bool
odEntityIndex_set_collider_impl(odEntityIndex* entity_index, odEntityCollider* old_collider, const odEntityCollider* collider);
static OD_NO_DISCARD bool
odEntityIndex_set_sprite_impl(odEntityIndex* entity_index, odEntitySprite* old_sprite, const odEntitySprite* sprite);

bool odEntityIndexEntity_check_valid(const odEntityIndexEntity* entity) {
	if (!OD_CHECK(entity != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odEntity_check_valid(&entity->entity))) {
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
	const odEntityChunkCoord coord_bitmask =
		static_cast<odEntityChunkCoord>((1 << OD_ENTITY_CHUNK_COORD_MASK_BITS) - 1);

	// in edge-case where width/height is big enough to exceed wrap-around, cover the full range
	if ((bounds.x2 - bounds.x1) >= (1 << OD_ENTITY_CHUNK_COORD_BITS)) {
		x2 = static_cast<odEntityChunkCoord>((x1 - 1) & coord_bitmask);
	}
	if ((bounds.y2 - bounds.y1) >= (1 << OD_ENTITY_CHUNK_COORD_BITS)) {
		y2 = static_cast<odEntityChunkCoord>((y1 - 1) & coord_bitmask);
	}

	// in edge-case where bounds are empty, set empty chunk bounds
	if ((bounds.x2 == bounds.x1) || (bounds.y2 == bounds.y1)) {
		x2 = x1;
		y2 = y1;
	}
}

odEntityChunkCoord odChunkCoord_init(float value) {
	OD_DISCARD(OD_DEBUG_CHECK(odFloat_is_precise_int24(value)));
	const int32_t coord_bitmask =
		static_cast<int32_t>((1 << OD_ENTITY_CHUNK_COORD_MASK_BITS) - 1);
	return static_cast<odEntityChunkCoord>(
		(static_cast<int32_t>(value) >> OD_ENTITY_CHUNK_COORD_DISCARD_BITS) & coord_bitmask
	);
}
float odChunkCoord_get_value(odEntityChunkCoord coord) {
	return static_cast<float>(static_cast<int32_t>(coord) << OD_ENTITY_CHUNK_COORD_DISCARD_BITS);
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
	if (chunk->colliders.get_count() <= 64) {
		entities_str = odDebugString_format_array(
			reinterpret_cast<const char*(*)(const void*)>(&odEntity_get_debug_string),
			chunk->colliders.begin(),
			chunk->colliders.get_count(),
			sizeof(odEntityIndexEntity));
	}
	if (entities_str == nullptr) {
		entities_str = "\"...\"";
	}

	const char* entity_ids_str = nullptr;
	if (chunk->entity_ids.get_count() <= 64) {
		entity_ids_str = odDebugString_format_array(
			[](const void* entity_id) -> const char* {
				return odDebugString_format("%d", *reinterpret_cast<const odEntityId*>(entity_id));
			},
			chunk->entity_ids.begin(),
			chunk->entity_ids.get_count(),
			sizeof(odEntityIndexEntity));
	}
	if (entity_ids_str == nullptr) {
		entity_ids_str = "\"...\"";
	}

	return odDebugString_format(
		"{\"count\": %d, \"entities\": [%s], \"entity_ids\": [%s]}",
		chunk->colliders.get_count(),
		entities_str,
		entity_ids_str
	);
}
bool odEntityIndex_chunk_find_collider(odEntityIndex* entity_index, odEntityChunkId chunk_id, odEntityId entity_id, int32_t* out_chunk_index) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((chunk_id >= 0) && (chunk_id < OD_ENTITY_CHUNK_ID_COUNT))
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < entity_index->entities.get_count()))
		|| !OD_DEBUG_CHECK(out_chunk_index != nullptr)) {
		return false;
	}

	odEntityChunk& chunk = entity_index->chunks[chunk_id];
	int32_t chunk_index = 0;
	bool found = false;
	const odEntityId *chunk_entity_ids = chunk.entity_ids.begin();
	int32_t chunk_entity_count = chunk.entity_ids.get_count();

	for (int32_t i = 0; i < chunk_entity_count; i++) {
		if (chunk_entity_ids[i] == entity_id) {
			chunk_index = i;
			found = true;
			break;
		}
	}

	*out_chunk_index = chunk_index;

	return found;
}
bool odEntityIndex_chunk_unset_collider(odEntityIndex* entity_index, odEntityChunkId chunk_id, odEntityId entity_id) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((chunk_id >= 0) && (chunk_id < OD_ENTITY_CHUNK_ID_COUNT))
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < entity_index->entities.get_count()))) {
		return false;
	}

	odEntityChunk& chunk = entity_index->chunks[chunk_id];
	int32_t chunk_index = 0;
	if (!OD_CHECK(odEntityIndex_chunk_find_collider(entity_index, chunk_id, entity_id, &chunk_index))) {
		return false;
	}

	if (!OD_DEBUG_CHECK((chunk_index >= 0) && (chunk_index < chunk.colliders.get_count()))) {
		return false;
	}

	if (!OD_CHECK(chunk.colliders.swap_pop(chunk_index))) {
		return false;
	}

	if (!OD_CHECK(chunk.entity_ids.swap_pop(chunk_index))) {
		return false;
	}

	if (!OD_DEBUG_CHECK((chunk.entity_ids.get_count() == chunk.colliders.get_count()))) {
		return false;
	}

	return true;
}
bool odEntityIndex_chunk_set_collider(odEntityIndex* entity_index, odEntityChunkId chunk_id, const odEntityCollider* collider) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((chunk_id >= 0) && (chunk_id < OD_ENTITY_CHUNK_ID_COUNT))
		|| !OD_DEBUG_CHECK(odEntityCollider_check_valid(collider))
		|| !OD_DEBUG_CHECK(odBounds_has_area(&collider->bounds))
		|| !OD_DEBUG_CHECK(collider->id < entity_index->entities.get_count())) {
		return false;
	}

	odEntityChunk& chunk = entity_index->chunks[chunk_id];
	int32_t chunk_index = 0;
	if (!odEntityIndex_chunk_find_collider(entity_index, chunk_id, collider->id, &chunk_index)) {
		if (!OD_CHECK(chunk.colliders.extend(collider, 1))) {
			return false;
		}

		if (!OD_CHECK(chunk.entity_ids.extend(&collider->id, 1))) {
			return false;
		}

		return true;
	}

	if (!OD_DEBUG_CHECK((chunk_index >= 0) && (chunk_index < chunk.colliders.get_count()))) {
		return false;
	}

	if (!OD_DEBUG_CHECK((chunk.entity_ids.get_count() == chunk.colliders.get_count()))) {
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

	int32_t old_count = entity_index->entities.get_count();
	if (min_count > old_count) {
		if (!OD_CHECK(entity_index->entities.ensure_count(min_count))) {
			return false;
		}

		for (int32_t i = old_count; i < min_count; i++) {
			entity_index->entities[i].entity.collider.id = i;
		}
	}

	int32_t min_vertices_count = min_count * 6;
	if (!OD_CHECK(entity_index->entity_vertices.ensure_count(min_vertices_count))) {
		return false;
	}

	return true;
}
odEntityIndexEntity* odEntityIndex_get_or_add_allocation(odEntityIndex* entity_index, odEntityId entity_id) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(entity_id >= 0)) {
		return nullptr;
	}

	if (!OD_CHECK(odEntityIndex_ensure_count(entity_index, entity_id + 1))) {
		return nullptr;
	}

	odEntityIndexEntity* entity_allocation = entity_index->entities.get(entity_id);
	if (!OD_DEBUG_CHECK(odEntityIndexEntity_check_valid(entity_allocation))
		|| !OD_DEBUG_CHECK(entity_allocation->entity.collider.id == entity_id)) {
		return nullptr;
	}

	return entity_allocation;
}
bool odEntityIndex_update_vertices_impl(odEntityIndex* entity_index, const odEntityIndexEntity* entity) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(odEntityIndexEntity_check_valid(entity))) {
		return false;
	}

	odSpritePrimitive sprite{};
	odEntity_get_sprite(&entity->entity, &sprite);

	int32_t vertex_index = static_cast<int32_t>(entity->entity.collider.id) * OD_SPRITE_VERTEX_COUNT;

	if (!OD_DEBUG_CHECK((vertex_index + OD_SPRITE_VERTEX_COUNT) <= entity_index->entity_vertices.get_count())) {
		return false;
	}

	odVertex* vertices = entity_index->entity_vertices.get(vertex_index);
	if (!OD_DEBUG_CHECK(odVertex_check_valid_batch_3d(vertices, OD_SPRITE_VERTEX_COUNT))) {
		return false;
	}

	odSpritePrimitive_get_vertices(&sprite, vertices);

	for (int32_t i = 0; i < OD_ENTITY_VERTEX_COUNT; i++) {
		odVertex_transform_3d(vertices + i, &entity->entity.sprite.transform);
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

	if ((entity_index->entities.begin() != nullptr) && (entity_index->entities.get_count() <= 64)) {
		entities_str = odDebugString_format_array(
			reinterpret_cast<const char*(*)(const void*)>(&odEntity_get_debug_string),
			entity_index->entities.begin(),
			entity_index->entities.get_count(),
			sizeof(odEntityIndexEntity));
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
		entity_index->entities.get_count(),
		entities_str,
		chunks_str);
}
void odEntityIndex_init(odEntityIndex* entity_index) {
	OD_DEBUG("entity_index=%p", static_cast<const void*>(entity_index));

	if (!OD_CHECK(entity_index != nullptr)) {
		return;
	}

	odEntityIndex_destroy(entity_index);
}
void odEntityIndex_destroy(odEntityIndex* entity_index) {
	OD_DEBUG("entity_index=%p", static_cast<const void*>(entity_index));

	if (!OD_CHECK(entity_index != nullptr)) {
		return;
	}

	odTrivialArray_destroy(&entity_index->entities);
	odTrivialArray_destroy(&entity_index->entity_vertices);
	for (odEntityChunkId i = 0; i < OD_ENTITY_CHUNK_ID_COUNT; i++) {
		odTrivialArray_destroy(&entity_index->chunks[i].colliders);
	}
}
odEntityId odEntityIndex_get_count(const odEntityIndex* entity_index) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return false;
	}

	return entity_index->entities.get_count();
}
const struct odVertex* odEntityIndex_get_vertices(const odEntityIndex* entity_index, odEntityId entity_id) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < entity_index->entities.get_count()))) {
		return nullptr;
	}

	int32_t vertex_index = static_cast<int32_t>(entity_id) * 6;
	if (!OD_DEBUG_CHECK(vertex_index < entity_index->entity_vertices.get_count())) {
		return nullptr;
	}

	const odVertex* vertices = entity_index->entity_vertices.get(vertex_index);
	if (!OD_DEBUG_CHECK(odVertex_check_valid_batch_3d(vertices, OD_SPRITE_VERTEX_COUNT))) {
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

	if (entity_index->entity_vertices.get_count() == 0) {
		return nullptr;
	}

	const odVertex* vertices = entity_index->entity_vertices.begin();
	if (!OD_CHECK(vertices != nullptr)) {
		return nullptr;
	}

	*out_vertex_count = entity_index->entity_vertices.get_count();
	return vertices;
}
const odEntity* odEntityIndex_get(const odEntityIndex* entity_index, odEntityId entity_id) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < entity_index->entities.get_count()))) {
		return nullptr;
	}

	return &entity_index->entities[entity_id].entity;
}
const odEntity* odEntityIndex_get_or_add(odEntityIndex* entity_index, odEntityId entity_id) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((entity_id >= 0))) {
		return nullptr;
	}

	const odEntityIndexEntity* entity = odEntityIndex_get_or_add_allocation(entity_index, entity_id);
	return &entity->entity;
}
void odEntityIndex_set_collider(odEntityIndex* entity_index, const odEntityCollider* collider) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(odEntityCollider_check_valid(collider))) {
		return;
	}

	odEntityIndexEntity* old_entity = odEntityIndex_get_or_add_allocation(entity_index, collider->id);
	if (!OD_DEBUG_CHECK(odEntityIndexEntity_check_valid(old_entity))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_set_collider_impl(entity_index, &old_entity->entity.collider, collider))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_update_vertices_impl(entity_index, old_entity))) {
		return;
	}
}
void odEntityIndex_set_sprite(odEntityIndex* entity_index, odEntityId entity_id, const odEntitySprite* sprite) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK((entity_id >= 0) && (entity_id < entity_index->entities.get_count()))
		|| !OD_DEBUG_CHECK(odEntitySprite_check_valid(sprite))) {
		return;
	}

	odEntityIndexEntity* old_entity = odEntityIndex_get_or_add_allocation(entity_index, entity_id);
	if (!OD_DEBUG_CHECK(odEntityIndexEntity_check_valid(old_entity))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_set_sprite_impl(entity_index, &old_entity->entity.sprite, sprite))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_update_vertices_impl(entity_index, old_entity))) {
		return;
	}
}
void odEntityIndex_set(odEntityIndex* entity_index, const odEntity* entity) {
	if (!OD_DEBUG_CHECK(entity_index != nullptr)
		|| !OD_DEBUG_CHECK(odEntity_check_valid(entity))) {
		return;
	}

	odEntityIndexEntity* old_entity = odEntityIndex_get_or_add_allocation(entity_index, entity->collider.id);
	if (!OD_DEBUG_CHECK(odEntityIndexEntity_check_valid(old_entity))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_set_collider_impl(entity_index, &old_entity->entity.collider, &entity->collider))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_set_sprite_impl(entity_index, &old_entity->entity.sprite, &entity->sprite))) {
		return;
	}

	if (!OD_CHECK(odEntityIndex_update_vertices_impl(entity_index, old_entity))) {
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

			if ((search->opt_exclude_entity_id != nullptr) && (*search->opt_exclude_entity_id == collider.id)) {
				continue;
			}

			// if seen on previous x or y coord, do not add
			if (search->max_results > 1) {
				odEntityChunkCoord coord_x = odEntityChunkId_get_coord_x(chunk_id);
				odEntityChunkCoord coord_y = odEntityChunkId_get_coord_y(chunk_id);
				float x = odChunkCoord_get_value(coord_x);
				float y = odChunkCoord_get_value(coord_y);
				float x_left = x - 1;
				float y_up = y - 1;

				if ((coord_x != search_bounds.x_start)
					&& (x_left >= collider.bounds.x1)
					&& (x_left <= collider.bounds.x2)) {
					continue;
				}

				if ((coord_y != search_bounds.y_start)
					&& (y_up >= collider.bounds.y1)
					&& (y_up <= collider.bounds.y2)) {
					continue;
				}
			}

			int32_t index = count++;
			if (search->opt_out_results == nullptr) {
				continue;
			}

			search->opt_out_results[index] = collider.id;
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
		"{\"opt_out_results\": \"%p\", \"max_results\": %d, \"tagset\": %s, \"bounds\": %s}",
		static_cast<const void*>(search->opt_out_results),
		search->max_results,
		odTagset_get_debug_string(&search->tagset),
		odBounds_get_debug_string(&search->bounds));
}
bool odEntitySearch_check_valid(const odEntitySearch* search) {
	if (!OD_CHECK(search != nullptr)) {
		return false;
	}

	if (!OD_CHECK(search->max_results >= 0)
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

static_assert(
	OD_ENTITY_CHUNK_COORD_MASK_BITS <= (8 * sizeof(odEntityChunkCoord)),
	"chunk coord must fit chunk coord type");
static_assert(
	OD_ENTITY_CHUNK_ID_BITS <= 14,
	"chunk count must not exceed 2^14 chunks (this limit is already excessive)");
