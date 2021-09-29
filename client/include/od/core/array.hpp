#pragma once

#include <od/core/array.h>

#include <od/core/type.hpp>
#include <od/core/allocation.hpp>

struct odArray {
	struct odAllocation allocation;
	const struct odType* type;
	int32_t capacity;
	int32_t count;

	OD_ENGINE_CORE_MODULE odArray();
	OD_ENGINE_CORE_MODULE explicit odArray(const odType* in_type);
	OD_ENGINE_CORE_MODULE odArray(odArray&& other);
	OD_ENGINE_CORE_MODULE odArray& operator=(odArray&& other);
	OD_ENGINE_CORE_MODULE ~odArray();

	odArray(const odArray& other) = delete;
	odArray& operator=(const odArray& other) = delete;
};
