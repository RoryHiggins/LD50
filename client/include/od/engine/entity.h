#pragma once

#include <od/engine/module.h>

#include <od/core/color.h>
#include <od/core/bounds.h>
#include <od/core/matrix.h>
#include <od/engine/tagset.h>

typedef int32_t odEntityId;

struct odSpritePrimitive;

struct odEntityCollider {
	odEntityId id;
	struct odBounds bounds;
	struct odTagset tagset;
};
struct odEntitySprite {
	struct odBounds texture_bounds;
	struct odColor color;
	float depth;
	struct odMatrix transform;
};
struct odEntity {
	struct odEntityCollider collider;
	struct odEntitySprite sprite;
};

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const char*
odEntityCollider_get_debug_string(const struct odEntityCollider* collider);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntityCollider_check_valid(const struct odEntityCollider* collider);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntityCollider_get_equals(const struct odEntityCollider* a, const struct odEntityCollider* b);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const char*
odEntitySprite_get_debug_string(const struct odEntitySprite* sprite);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntitySprite_check_valid(const struct odEntitySprite* sprite);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD const char*
odEntity_get_debug_string(const struct odEntity* entity);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEntity_check_valid(const struct odEntity* entity);
OD_API_C OD_ENGINE_MODULE void
odEntity_get_sprite(const struct odEntity* entity, struct odSpritePrimitive *out_sprite);
