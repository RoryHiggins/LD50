#pragma once

#include <od/core/string.h>

#include <od/core/allocation.hpp>

struct odString {
	struct odAllocation allocation;
	int32_t capacity;
	int32_t count;

	OD_CORE_MODULE odString();
	OD_CORE_MODULE odString(odString&& other);
	OD_CORE_MODULE odString(const odString& other);
	OD_CORE_MODULE odString& operator=(odString&& other);
	OD_CORE_MODULE odString& operator=(const odString& other);
	OD_CORE_MODULE ~odString();

	OD_CORE_MODULE char* operator[](int32_t i) &;
	OD_CORE_MODULE const char* operator[](int32_t i) const&;
	OD_CORE_MODULE char* begin() &;
	OD_CORE_MODULE const char* begin() const&;
	OD_CORE_MODULE char* end() &;
	OD_CORE_MODULE const char* end() const&;

	// prevent member access when expiring
	char* operator[](int32_t i) && = delete;
	const char* operator[](int32_t i) const&& = delete;
	char* begin() && = delete;
	const char* begin() const&& = delete;
	char* end() && = delete;
	const char* end() const&& = delete;
};
