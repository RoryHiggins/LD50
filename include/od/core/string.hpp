#pragma once

#include <od/core/string.h>

#include <od/core/array.hpp>

struct odString {
	struct odArray array;

	OD_API_CPP odString();
	OD_API_CPP odString(odString&& other);
	OD_API_CPP odString(const odString& other);
	OD_API_CPP odString& operator=(odString&& other);
	OD_API_CPP odString& operator=(const odString& other);
	OD_API_CPP ~odString();
};
