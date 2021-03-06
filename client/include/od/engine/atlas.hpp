#pragma once

#include <od/engine/atlas.h>

#include <od/core/bounds.h>
#include <od/core/array.hpp>
#include <od/platform/image.hpp>

struct odAtlasRegion {
	odBounds bounds;
};

struct odAtlas {
	odImage image;
	odTrivialArrayT<odAtlasRegion> regions;  // by region id
	odTrivialArrayT<odAtlasRegion> free_regions;

	OD_ENGINE_MODULE odAtlas();
	OD_ENGINE_MODULE odAtlas(odAtlas&& other);
	OD_ENGINE_MODULE odAtlas& operator=(odAtlas&& other);
	OD_ENGINE_MODULE ~odAtlas();

	odAtlas(const odAtlas& other) = delete;
	odAtlas& operator=(const odAtlas& other) = delete;
};
