#pragma once

#include <od/engine/world.hpp>

#include <od/platform/vertex.h>
#include <od/engine/entity.hpp>

#define OD_ENTITY_STORAGE_AOSOA_BITS 3
#define OD_ENTITY_STORAGE_AOSOA_COUNT (1 << OD_ENTITY_STORAGE_AOSOA_BITS)
#define OD_ENTITY_STORAGE_AOSOA_SUBINDEX_MASK (OD_ENTITY_STORAGE_AOSOA_COUNT - 1)

struct odInt32Pair {
	int32_t a;
	int32_t b;
};

struct odEntitySprite {
	float depth;
	odColor color;
	odBounds texture_bounds;
	odMatrix matrix;
};
struct odEntityStorage {
	odTagset tagset[OD_ENTITY_STORAGE_AOSOA_COUNT];
	int32_t x[OD_ENTITY_STORAGE_AOSOA_COUNT];
	int32_t y[OD_ENTITY_STORAGE_AOSOA_COUNT];
	int32_t width[OD_ENTITY_STORAGE_AOSOA_COUNT];
	int32_t height[OD_ENTITY_STORAGE_AOSOA_COUNT];
	
	odEntitySprite sprite[OD_ENTITY_STORAGE_AOSOA_COUNT];

	odArrayT</*chunk_index, map_index*/ odInt32Pair> chunk_entity_map_indices[OD_ENTITY_STORAGE_AOSOA_COUNT];
	odArrayT</*tag_index, map_index*/ odInt32Pair> tag_entity_map_indices[OD_ENTITY_STORAGE_AOSOA_COUNT];
};

struct odChunkIterator {
	uint8_t x1;
	uint8_t y1;
	uint8_t x2;
	uint8_t y2;

	explicit odChunkIterator(odBounds bounds);

	odChunkIterator begin() const;
	odChunkIterator end() const;
	bool operator==(const odChunkIterator& other) const;
	bool operator!=(const odChunkIterator& other) const;
	odChunkIterator& operator++();
	odChunkIterator& operator++(int);
	/*chunk_index*/ int32_t operator*();
};

template struct odArrayT<odInt32Pair>;

static uint8_t odChunk_get_coord(float coord);
static int32_t odChunk_get_index(float x, float y);



bool odWorld_init(struct odWorld* world) {
	OD_DEBUG("world=%p", static_cast<const void*>(world));

	if (!OD_DEBUG_CHECK(world != nullptr)) {
		return false;
	}

	odWorld_destroy(world);

	return true;
}
void odWorld_destroy(struct odWorld* world) {
	OD_DEBUG("world=%p", static_cast<const void*>(world));

	if (!OD_DEBUG_CHECK(world != nullptr)) {
		return;
	}

	odArray_destroy(&world->entity_storage);

	for (int32_t i = 0; i < OD_WORLD_CHUNK_INDEX_MAX; i++) {
		odArray_destroy(&world->chunk_entity_map[i]);
	}

	for (int32_t i = 0; i < OD_TAG_INDEX_MAX; i++) {
		odArray_destroy(&world->tag_entity_map[i]);
	}
}
static OD_NO_DISCARD bool odWorld_unset_bounds(struct odWorld* world, int32_t entity_index) {
	odEntityStorage* storage = world->entity_storage[entity_index >> OD_ENTITY_STORAGE_AOSOA_BITS];
	int32_t storage_subindex = entity_index & OD_ENTITY_STORAGE_AOSOA_SUBINDEX_MASK;

	for (odInt32Pair chunk_map_index_pair: storage->chunk_entity_map_indices[storage_subindex]) {
		int32_t chunk_index = chunk_map_index_pair.a;
		int32_t map_index = chunk_map_index_pair.b;

		odArrayT</*entity_index*/ int32_t>* chunk_entity_array = &world->chunk_entity_map[chunk_index];

		if (!OD_CHECK(odArray_swap_pop(chunk_entity_array, map_index))) {
			return false;
		}

		int32_t chunk_entity_swapped_index = odArray_get_count(chunk_entity_array) - 1;

		if (chunk_entity_swapped_index >= 0) {
			// update indexing for entity swapped in the chunk's entity index
			// unfortunately messy due to AoSoA layout and long confusing names
			int32_t swapped_entity_index = *(*chunk_entity_array)[chunk_entity_swapped_index];
			odEntityStorage* swapped_storage = world->entity_storage[swapped_entity_index >> OD_ENTITY_STORAGE_AOSOA_BITS];
			int32_t swapped_storage_subindex = swapped_entity_index & OD_ENTITY_STORAGE_AOSOA_SUBINDEX_MASK;
			
			odInt32Pair* swapped_chunk_entity_map_index_array =
				swapped_storage->chunk_entity_map_indices[swapped_storage_subindex].begin();
			int32_t swapped_chunk_entity_map_index_count = odArray_get_count(
				&swapped_storage->chunk_entity_map_indices[swapped_storage_subindex]);
			
			bool swapped_chunk_entity_map_index_found = false;
			for (int32_t i = 0; i < swapped_chunk_entity_map_index_count; i++) {
				if (swapped_chunk_entity_map_index_array[i].a == chunk_index) {
					swapped_chunk_entity_map_index_array[i].b = chunk_entity_swapped_index;
					swapped_chunk_entity_map_index_found = true;
					break;
				}
			}

			if (!OD_DEBUG_CHECK(swapped_chunk_entity_map_index_found)) {
				return false;
			}
			OD_MAYBE_UNUSED(swapped_chunk_entity_map_index_found);
		}
	}

	if (!odArray_set_count(&storage->chunk_entity_map_indices[storage_subindex], 0)) {
		return false;
	}

	storage->x[storage_subindex] = 0;
	storage->y[storage_subindex] = 0;
	storage->width[storage_subindex] = 0;
	storage->height[storage_subindex] = 0;

	return true;
}
static OD_NO_DISCARD bool odWorld_unset_tag(struct odWorld* world, int32_t entity_index, int32_t tag_index) {
	odEntityStorage* storage = world->entity_storage[entity_index >> OD_ENTITY_STORAGE_AOSOA_BITS];
	int32_t storage_subindex = entity_index & OD_ENTITY_STORAGE_AOSOA_SUBINDEX_MASK;

	if (!odTagset_get(&storage->tagset[storage_subindex], tag_index)) {
		return true;
	}

	bool tag_map_index_found = false;
	int32_t tag_map_index = 0;
	odInt32Pair* tag_entity_map_index_array =
		storage->tag_entity_map_indices[storage_subindex].begin();
	int32_t tag_entity_map_index_count = odArray_get_count(
		&storage->tag_entity_map_indices[storage_subindex]);

	for (int32_t i = 0; i < tag_entity_map_index_count; i++) {
		if (tag_entity_map_index_array[i].a == tag_index) {
			tag_map_index_found = true;
			tag_map_index = i;
			break;
		}
	}

	if (!OD_DEBUG_CHECK(tag_map_index_found)) {
		return false;
	}
	OD_MAYBE_UNUSED(tag_map_index_found);

	odInt32Pair tag_map_index_pair = *storage->tag_entity_map_indices[storage_subindex][tag_map_index];
	int32_t map_index = tag_map_index_pair.b;

	odArrayT</*entity_index*/ int32_t>* tag_entity_array = &world->tag_entity_map[tag_index];

	if (!OD_CHECK(odArray_swap_pop(tag_entity_array, map_index))) {
		return false;
	}

	int32_t swapped_tag_entity_index = odArray_get_count(tag_entity_array) - 1;

	if (swapped_tag_entity_index >= 0) {
		// update indexing for entity swapped in the chunk's entity index
		// unfortunately messy due to AoSoA layout and long confusing names
		int32_t swapped_entity_index = *(*tag_entity_array)[swapped_tag_entity_index];
		odEntityStorage* swapped_storage = world->entity_storage[swapped_entity_index >> OD_ENTITY_STORAGE_AOSOA_BITS];
		int32_t swapped_storage_subindex = swapped_entity_index & OD_ENTITY_STORAGE_AOSOA_SUBINDEX_MASK;
		
		odInt32Pair* swapped_tag_entity_map_index_array =
			swapped_storage->tag_entity_map_indices[swapped_storage_subindex].begin();
		int32_t swapped_tag_entity_map_index_count = odArray_get_count(
			&swapped_storage->tag_entity_map_indices[swapped_storage_subindex]);
		
		bool swapped_tag_entity_map_index_found = false;
		for (int32_t i = 0; i < swapped_tag_entity_map_index_count; i++) {
			if (swapped_tag_entity_map_index_array[i].a == tag_index) {
				swapped_tag_entity_map_index_array[i].b = swapped_tag_entity_index;
				swapped_tag_entity_map_index_found = true;
				break;
			}
		}

		if (!OD_DEBUG_CHECK(swapped_tag_entity_map_index_found)) {
			return false;
		}
		OD_MAYBE_UNUSED(swapped_tag_entity_map_index_found);
	}

	if (!OD_CHECK(odArray_swap_pop(&storage->tag_entity_map_indices[storage_subindex], tag_map_index))) {
		return false;
	}

	odTagset_set(&storage->tagset[storage_subindex], tag_index, false);

	return true;
}
static OD_NO_DISCARD bool odWorld_unset_tags(struct odWorld* world, int32_t entity_index) {
	for (int32_t tag_index = 0; tag_index < OD_TAG_INDEX_MAX; tag_index++) {
		if (!odWorld_unset_tag(world, entity_index, tag_index)) {
			return false;
		}
	}
	return true;
}
bool odWorld_unset(struct odWorld* world, int32_t entity_index) {
	if (!OD_DEBUG_CHECK(world != nullptr)
		|| !OD_DEBUG_CHECK(entity_index >= 0)
		|| !OD_DEBUG_CHECK((entity_index >> OD_ENTITY_STORAGE_AOSOA_BITS) >= odArray_get_count(&world->entity_storage))
		|| !OD_CHECK(odWorld_unset_bounds(world, entity_index))
		|| !OD_CHECK(odWorld_unset_tags(world, entity_index))) {
		return false;
	}

	odEntityStorage* storage = world->entity_storage[entity_index >> OD_ENTITY_STORAGE_AOSOA_BITS];
	int32_t storage_subindex = entity_index & OD_ENTITY_STORAGE_AOSOA_SUBINDEX_MASK;
	if (!OD_DEBUG_CHECK(storage != nullptr)) {
		return false;
	}

	storage->sprite[storage_subindex] = odEntitySprite{};

	return true;
}
// bool odWorld_set(struct odWorld* world, int32_t entity_index, const struct odEntity* entity) {

// }
// bool odWorld_set_bounds(struct odWorld* world, int32_t entity_index, const struct odBounds* bounds) {

// }
// const struct odEntity* odWorld_get(const struct odWorld* world, int32_t entity_index) {

// }
// int32_t odWorld_search_init(const struct odWorld* world, const struct odWorldSearch* search,
// 							struct odWorldIterator* out_iterator) {

// }
// bool odWorld_search(const struct odWorld* world, struct odWorldIterator* iterator,
// 					int32_t* out_entity_index) {

// }

odWorld::odWorld() = default;
odWorld::odWorld(odWorld&& other) = default;
odWorld& odWorld::operator=(odWorld&& other) = default;
odWorld::~odWorld() = default;

odChunkIterator::odChunkIterator(odBounds bounds)
	: x1{odChunk_get_coord(bounds.x)}, y1{odChunk_get_coord(bounds.y)},
	  x2{odChunk_get_coord(bounds.x + bounds.width)}, y2{odChunk_get_coord(bounds.y + bounds.width)} {
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
/*chunk_index*/ int32_t odChunkIterator::operator*() {
	return (
		static_cast<int32_t>(x1)
		+ static_cast<int32_t>(y1 << OD_WORLD_CHUNK_COORD_MASK_BITS));
}

static uint8_t odChunk_get_coord(float coord) {
	const int32_t coord_bitmask = (1 << OD_WORLD_CHUNK_COORD_MASK_BITS) - 1;
	return (static_cast<int32_t>(coord) >> OD_WORLD_CHUNK_COORD_DISCARD_BITS) & coord_bitmask;
}
static int32_t odChunk_get_index(float x, float y) {
	return (
		static_cast<int32_t>(odChunk_get_coord(x))
		+ static_cast<int32_t>((odChunk_get_coord(y) << OD_WORLD_CHUNK_COORD_MASK_BITS)));
}

template struct odArrayT<odEntityStorage>;
