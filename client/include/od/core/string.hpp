#pragma once

#include <od/core/string.h>

#include <od/core/array.hpp>

struct odString {
	struct odArray array;

	OD_CORE_MODULE odString();
	OD_CORE_MODULE odString(odString&& other);
	OD_CORE_MODULE odString(const odString& other);
	OD_CORE_MODULE odString& operator=(odString&& other);
	OD_CORE_MODULE odString& operator=(const odString& other);
	OD_CORE_MODULE ~odString();
};
