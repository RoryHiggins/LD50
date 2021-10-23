#pragma once

#include <od/platform/render_texture.h>

#include <od/platform/texture.hpp>

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
