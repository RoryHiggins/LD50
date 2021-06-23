#pragma once

#include <od/core/box.h>

#include <od/core/allocation.hpp>

struct odType;

struct odBox {
	struct odAllocation allocation;
	const odType* type;

	OD_ENGINE_CORE_MODULE odBox();
	OD_ENGINE_CORE_MODULE explicit odBox(const odType* in_type);
	OD_ENGINE_CORE_MODULE odBox(odBox&& other);
	OD_ENGINE_CORE_MODULE odBox& operator=(odBox&& other);
	OD_ENGINE_CORE_MODULE ~odBox();

	odBox(const odBox& other) = delete;
	odBox& operator=(const odBox& other) = delete;
};
