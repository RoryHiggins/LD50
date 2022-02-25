#pragma once

#include <od/platform/file.h>

#include <od/core/string.hpp>

struct odFile {
	void* native_file;

	OD_PLATFORM_MODULE odFile();
	OD_PLATFORM_MODULE odFile(odFile&& other);
	OD_PLATFORM_MODULE odFile& operator=(odFile&& other);
	OD_PLATFORM_MODULE ~odFile();

	odFile(const odFile& other) = delete;
	odFile& operator=(const odFile& other) = delete;
};

struct odScopedTempFile {
	odString filename;

	OD_PLATFORM_MODULE explicit odScopedTempFile(odString in_filename);
	OD_PLATFORM_MODULE ~odScopedTempFile();

	odScopedTempFile(odScopedTempFile&& other) = delete;
	odScopedTempFile(const odScopedTempFile& other) = delete;
	odScopedTempFile& operator=(odScopedTempFile&& other) = delete;
	odScopedTempFile& operator=(const odScopedTempFile& other) = delete;
};
