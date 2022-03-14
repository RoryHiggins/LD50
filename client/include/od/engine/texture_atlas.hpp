#pragma once

#include <od/engine/texture_atlas.h>

#include <od/platform/texture.hpp>
#include <od/engine/atlas.hpp>

struct odTextureAtlas {
	odTexture texture;
	odAtlas atlas;

	OD_ENGINE_MODULE odTextureAtlas();
	OD_ENGINE_MODULE odTextureAtlas(odTextureAtlas&& other);
	OD_ENGINE_MODULE odTextureAtlas& operator=(odTextureAtlas&& other);
	OD_ENGINE_MODULE ~odTextureAtlas();

	odTextureAtlas(const odTextureAtlas& other) = delete;
	odTextureAtlas& operator=(const odTextureAtlas& other) = delete;
};
