#pragma once

#include <od/engine/entity.hpp>

#include <od/core/debug.h>

bool odEntity_get_valid(const odEntity* entity) {
	if ((entity->id < 0)
		|| (!odBounds_get_valid(&entity->bounds))) {
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

template struct odArrayT<odEntity>;
