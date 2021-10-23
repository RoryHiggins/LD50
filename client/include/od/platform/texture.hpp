#pragma once

#include <od/platform/texture.h>

struct odTexture {
	void* render_context_native;
	uint32_t texture;

	OD_PLATFORM_MODULE odTexture();
	OD_PLATFORM_MODULE odTexture(odTexture&& other);
	OD_PLATFORM_MODULE odTexture& operator=(odTexture&& other);
	OD_PLATFORM_MODULE ~odTexture();

	odTexture(odTexture const& other) = delete;
	odTexture& operator=(const odTexture& other) = delete;
};
