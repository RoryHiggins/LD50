#pragma once

#include <od/platform/renderer.h>

#include <od/platform/window.hpp>

struct odRenderer : odWindowResource {
	uint32_t vbo;
	uint32_t vao;
	uint32_t vertex_shader;
	uint32_t fragment_shader;
	uint32_t program;
	uint32_t program_view_uniform;
	uint32_t program_projection_uniform;
	uint32_t program_uv_scale_uniform;

	OD_PLATFORM_MODULE odRenderer();
	OD_PLATFORM_MODULE odRenderer(odRenderer&& other);
	OD_PLATFORM_MODULE odRenderer& operator=(odRenderer&& other);
	OD_PLATFORM_MODULE ~odRenderer();

	odRenderer(odRenderer const& other) = delete;
	odRenderer& operator=(const odRenderer& other) = delete;
};
