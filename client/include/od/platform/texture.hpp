#pragma once

#include <od/platform/texture.h>

struct odTexture {
	odWindow* window;

	uint32_t texture;
	int32_t width;
	int32_t height;

	OD_PLATFORM_MODULE odTexture();
	OD_PLATFORM_MODULE odTexture(odTexture&& other);
	OD_PLATFORM_MODULE odTexture& operator=(odTexture&& other);
	OD_PLATFORM_MODULE ~odTexture();

	// copy _could_ be wrapped for convenience, but it's likely to be used
	// accidentally in c++, and a blocking texture copy can be very expensive
	odTexture(odTexture const& other) = delete;
	odTexture& operator=(const odTexture& other) = delete;
};
struct odRenderTexture {
	odTexture texture;

	uint32_t fbo;

	OD_PLATFORM_MODULE odRenderTexture();
	OD_PLATFORM_MODULE odRenderTexture(odRenderTexture&& other);
	OD_PLATFORM_MODULE odRenderTexture& operator=(odRenderTexture&& other);
	OD_PLATFORM_MODULE ~odRenderTexture();

	odRenderTexture(odRenderTexture const& other) = delete;
	odRenderTexture& operator=(const odRenderTexture& other) = delete;
};
