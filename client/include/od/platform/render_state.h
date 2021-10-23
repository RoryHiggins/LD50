#pragma once

#include <od/platform/module.h>

#include <od/core/primitive.h>

struct odTexture;
struct odRenderTexture;

struct odRenderState {
	struct odTransform view;
	struct odTransform projection;
	struct odBounds viewport;
	const struct odTexture* src_texture;
	struct odRenderTexture* render_texture;
};
