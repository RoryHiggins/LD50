#include <od/engine/entity.hpp>

#include <cmath>

#include <od/core/debug.h>

const char* odEntityCollider_get_debug_string(const odEntityCollider* collider) {
	if (collider == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"id\": %d, \"tagset\": %s, \"bounds\": %s}",
		collider->id,
		odTagset_get_debug_string(&collider->tagset),
		odBounds_get_debug_string(&collider->bounds));
}
bool odEntityCollider_check_valid(const odEntityCollider* collider) {
	if (!OD_CHECK(collider != nullptr)) {
		return false;
	}

	odBounds floored_bounds = collider->bounds;
	odBounds_floor(&floored_bounds);

	if (!OD_CHECK(collider->id >= 0)
		|| !OD_CHECK(odBounds_check_valid(&collider->bounds))
		|| !OD_CHECK(odBounds_equals(&floored_bounds, &collider->bounds))) {
		return false;
	}

	return true;
}
bool odEntityCollider_equals(const odEntityCollider* a, const odEntityCollider* b) {
	if (!OD_DEBUG_CHECK(a != nullptr)
		|| !OD_DEBUG_CHECK(b != nullptr)) {
		return false;
	}

	if ((a->id != b->id)
		|| (!odTagset_equals(&a->tagset, &b->tagset))
		|| (!odBounds_equals(&a->bounds, &b->bounds))) {
		return false;
	}

	return true;
}

const char* odEntitySprite_get_debug_string(const odEntitySprite* sprite) {
	if (sprite == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"depth\": %g, \"color\": %s, \"texture_bounds\": %s, \"transform\": %s}",
		static_cast<double>(sprite->depth),
		odColor_get_debug_string(&sprite->color),
		odBounds_get_debug_string(&sprite->texture_bounds),
		odMatrix4_get_debug_string(&sprite->transform));
}
bool odEntitySprite_check_valid(const odEntitySprite* sprite) {
	if (!OD_DEBUG_CHECK(sprite != nullptr)) {
		return false;
	}

	odBounds floored_texture_bounds = sprite->texture_bounds;
	odBounds_floor(&floored_texture_bounds);

	if (!std::isfinite(sprite->depth)
		|| (!odBounds_check_valid(&sprite->texture_bounds))
		|| (!odBounds_equals(&floored_texture_bounds, &sprite->texture_bounds))
		|| (!odMatrix4_check_valid(&sprite->transform))) {
		return false;
	}

	return true;
}

const char* odEntity_get_debug_string(const odEntity* entity) {
	if (entity == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"collider\": %s, \"sprite\": %s}",
		odEntityCollider_get_debug_string(&entity->collider),
		odEntitySprite_get_debug_string(&entity->sprite));
}
bool odEntity_check_valid(const odEntity* entity) {
	if (!OD_CHECK(entity != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odEntityCollider_check_valid(&entity->collider))
		|| !OD_CHECK(odEntitySprite_check_valid(&entity->sprite))) {
		return false;
	}

	return true;
}

template struct odFastArrayT<odEntityCollider>;
template struct odArrayT<odEntityCollider>;
template struct odArrayT<odEntity>;
