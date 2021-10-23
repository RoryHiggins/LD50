#pragma once

#include <od/platform/renderer.h>

#include <od/platform/texture.hpp>

struct odRenderer {
	void* render_context_native;
	uint32_t vbo;
	uint32_t vao;
	uint32_t vertex_shader;
	uint32_t fragment_shader;
	uint32_t program;

	OD_PLATFORM_MODULE odRenderer();
	OD_PLATFORM_MODULE odRenderer(odRenderer&& other);
	OD_PLATFORM_MODULE odRenderer& operator=(odRenderer&& other);
	OD_PLATFORM_MODULE ~odRenderer();

	odRenderer(odRenderer const& other) = delete;
	odRenderer& operator=(const odRenderer& other) = delete;
};
