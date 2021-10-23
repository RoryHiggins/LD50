#pragma once

#include <od/core/containers.h>

struct odAllocation {
	void* ptr;

	OD_CORE_MODULE odAllocation();
	OD_CORE_MODULE odAllocation(odAllocation&& other);
	OD_CORE_MODULE odAllocation& operator=(odAllocation&& other);
	OD_CORE_MODULE ~odAllocation();

	odAllocation(const odAllocation& other) = delete;
	odAllocation& operator=(const odAllocation& other) = delete;
};

struct odArray {
	struct odAllocation allocation;
	const struct odType* type;
	int32_t capacity;
	int32_t count;

	OD_CORE_MODULE odArray();
	OD_CORE_MODULE explicit odArray(const odType* in_type);
	OD_CORE_MODULE odArray(odArray&& other);
	OD_CORE_MODULE odArray& operator=(odArray&& other);
	OD_CORE_MODULE ~odArray();

	odArray(const odArray& other) = delete;
	odArray& operator=(const odArray& other) = delete;
};

struct odString {
	struct odArray array;

	OD_CORE_MODULE odString();
	OD_CORE_MODULE odString(odString&& other);
	OD_CORE_MODULE odString(const odString& other);
	OD_CORE_MODULE odString& operator=(odString&& other);
	OD_CORE_MODULE odString& operator=(const odString& other);
	OD_CORE_MODULE ~odString();
};
