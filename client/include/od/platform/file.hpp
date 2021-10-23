#pragma once

#include <od/platform/file.h>

struct odFile {
	void* native_file;

	OD_PLATFORM_MODULE odFile();
	OD_PLATFORM_MODULE odFile(odFile&& other);
	OD_PLATFORM_MODULE odFile& operator=(odFile&& other);
	OD_PLATFORM_MODULE ~odFile();
};
