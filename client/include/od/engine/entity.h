#pragma once

#include <od/engine/module.h>

#include <od/core/color.h>
#include <od/core/bounds.h>
#include <od/core/matrix.h>
#include <od/core/array.hpp>
#include <od/engine/tagset.h>

struct odEntity {
	odTagset tagset;
	odBounds bounds;
	
	float depth;
	odColor color;
	odBounds texture_bounds;
	odMatrix matrix;
};
