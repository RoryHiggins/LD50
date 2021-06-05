#pragma once

#include <od/core/vector.h>

#include <od/core/allocation.hpp>

struct odVector {
	struct odAllocation allocation;
	const struct odType* type;
	uint32_t capacity;
	uint32_t count;

	OD_API_CPP odVector();
	OD_API_CPP explicit odVector(const odType* in_type);
	OD_API_CPP odVector(odVector&& other);
	OD_API_CPP odVector& operator=(odVector&& other);
	OD_API_CPP ~odVector();

	odVector(const odVector& other) = delete;
	odVector& operator=(const odVector& other) = delete;
};
