#pragma once

#include <od/core/allocation.h>

struct odAllocation {
	void* ptr;

	OD_CORE_MODULE odAllocation();
	OD_CORE_MODULE odAllocation(odAllocation&& other);
	OD_CORE_MODULE odAllocation& operator=(odAllocation&& other);
	OD_CORE_MODULE ~odAllocation();

	odAllocation(const odAllocation& other) = delete;
	odAllocation& operator=(const odAllocation& other) = delete;
};
