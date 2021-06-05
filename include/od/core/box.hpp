#pragma once

#include <od/core/box.h>

#include <od/core/allocation.hpp>

struct odType;

struct odBox {
	struct odAllocation allocation;
	const odType* type;

	OD_API_CPP odBox();
	OD_API_CPP explicit odBox(const odType* in_type);
	OD_API_CPP odBox(odBox&& other);
	OD_API_CPP odBox& operator=(odBox&& other);
	OD_API_CPP ~odBox();

	odBox(const odBox& other) = delete;
	odBox& operator=(const odBox& other) = delete;
};
