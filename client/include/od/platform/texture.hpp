#pragma once

#include <od/platform/texture.h>

#include <od/platform/window.hpp>

struct odTexture : odWindowResource {
	uint32_t texture;
	int32_t width;
	int32_t height;

	OD_PLATFORM_MODULE odTexture();
	OD_PLATFORM_MODULE odTexture(odTexture&& other);
	OD_PLATFORM_MODULE odTexture& operator=(odTexture&& other);
	OD_PLATFORM_MODULE ~odTexture();

	// copy _could_ be wrapped for convenience, but it's likely to be used
	// accidentally in c++, and a blocking texture copy command could be very expensive
	odTexture(odTexture const& other) = delete;
	odTexture& operator=(const odTexture& other) = delete;
};
