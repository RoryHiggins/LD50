#pragma once

#include <od/engine/module.h>

#include <od/core/color.h>
#include <od/core/bounds.h>
#include <od/core/matrix.h>
#include <od/core/array.hpp>
#include <od/engine/tagset.h>

struct odEntity {
	int32_t id;
	odTagset tagset;
	odBounds bounds;
};
struct odEntitySprite {
	float depth;
	odColor color;
	odBounds texture_bounds;
	odMatrix4 transform;
};

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const char*
odEntity_get_debug_string(const struct odEntity* entity);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntity_check_valid(const struct odEntity* entity);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntity_equals(const struct odEntity* a, const struct odEntity* b);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const char*
odEntitySprite_get_debug_string(const struct odEntitySprite* sprite);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntitySprite_check_valid(const struct odEntitySprite* sprite);
