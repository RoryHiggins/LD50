#pragma once

#include <od/core/allocation.h>

struct odAllocation {
	void* ptr;

	OD_API_CORE_CPP odAllocation();
	OD_API_CORE_CPP odAllocation(odAllocation&& other);
	OD_API_CORE_CPP odAllocation& operator=(odAllocation&& other);
	OD_API_CORE_CPP ~odAllocation();

	odAllocation(const odAllocation& other) = delete;
	odAllocation& operator=(const odAllocation& other) = delete;
};
