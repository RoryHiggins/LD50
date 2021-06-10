#pragma once

#include <od/core/array.h>

#include <od/core/allocation.hpp>

struct odArray {
	struct odAllocation allocation;
	const struct odType* type;
	uint32_t capacity;
	uint32_t count;

	OD_API_ENGINE_CPP odArray();
	OD_API_ENGINE_CPP explicit odArray(const odType* in_type);
	OD_API_ENGINE_CPP odArray(odArray&& other);
	OD_API_ENGINE_CPP odArray& operator=(odArray&& other);
	OD_API_ENGINE_CPP ~odArray();

	odArray(const odArray& other) = delete;
	odArray& operator=(const odArray& other) = delete;
};
