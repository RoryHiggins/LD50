#pragma once

#include <od/engine/entity.hpp>

#include <cmath>

#include <od/core/debug.h>

const char* odEntity_get_debug_string(const odEntity* entity) {
	if (entity == nullptr) {
		return "odEntity{this=nullptr}";
	}

	return odDebugString_format(
		"odEntity{this=%p, id=%d, tagset=%s, bounds=%s}",
		static_cast<const void*>(entity),
		entity->id,
		odTagset_get_debug_string(&entity->tagset),
		odBounds_get_debug_string(&entity->bounds));
}
bool odEntity_check_valid(const odEntity* entity) {
	if (!OD_CHECK(entity != nullptr)) {
		return false;
	}

	odBounds floored_bounds = entity->bounds;
	odBounds_floor(&floored_bounds);

	if (!OD_CHECK(entity->id >= 0)
		|| !OD_CHECK(odBounds_check_valid(&entity->bounds))
		|| !OD_CHECK(odBounds_equals(&floored_bounds, &entity->bounds))) {
		return false;
	}

	return true;
}
bool odEntity_equals(const odEntity* a, const odEntity* b) {
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
		return "odEntitySprite{this=nullptr}";
	}

	return odDebugString_format(
		"odEntitySprite{this=%p, depth=%g, color=%s, texture_bounds=%s, transform=%s}",
		static_cast<const void*>(sprite),
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

template struct odArrayT<odEntity>;
