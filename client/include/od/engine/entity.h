#pragma once

#include <od/engine/module.h>

#include <od/core/color.h>
#include <od/core/bounds.h>
#include <od/core/matrix.h>
#include <od/engine/tag.h>

#define OD_ENTITY_ID_GENERATION_BITS 8

typedef int32_t odEntityIndex;

struct odEntityKey {
	odTagset tags;
	odBounds bounds;
};
struct odEntitySprite {
	float depth;
	odColor color;
	odBounds texture_bounds;
	odMatrix matrix;
};
struct odEntity {
	odEntityKey key;
	odEntitySprite sprite;
};
