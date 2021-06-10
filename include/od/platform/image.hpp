#pragma once

#include <od/platform/image.h>

#include <od/core/allocation.hpp>

struct odImage {
	struct odAllocation allocation;
	uint32_t width;
	uint32_t height;

	OD_API_PLATFORM_CPP odImage();
	OD_API_PLATFORM_CPP odImage(odImage const& other);
	OD_API_PLATFORM_CPP odImage(odImage&& other);
	OD_API_PLATFORM_CPP odImage& operator=(const odImage& other);
	OD_API_PLATFORM_CPP odImage& operator=(odImage&& other);
	OD_API_PLATFORM_CPP ~odImage();
};
