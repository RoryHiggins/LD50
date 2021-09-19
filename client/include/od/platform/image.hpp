#pragma once

#include <od/platform/image.h>

#include <od/core/allocation.hpp>

struct odImage {
	struct odAllocation allocation;
	int32_t width;
	int32_t height;

	OD_ENGINE_PLATFORM_MODULE odImage();
	OD_ENGINE_PLATFORM_MODULE odImage(odImage const& other);
	OD_ENGINE_PLATFORM_MODULE odImage(odImage&& other);
	OD_ENGINE_PLATFORM_MODULE odImage& operator=(const odImage& other);
	OD_ENGINE_PLATFORM_MODULE odImage& operator=(odImage&& other);
	OD_ENGINE_PLATFORM_MODULE ~odImage();
};
