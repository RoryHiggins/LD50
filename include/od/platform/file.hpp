#pragma once

#include <od/platform/file.h>

struct odFile {
	void* native_file;

	OD_API_ENGINE_CPP odFile();
	OD_API_ENGINE_CPP odFile(odFile&& other);
	OD_API_ENGINE_CPP odFile& operator=(odFile&& other);
	OD_API_ENGINE_CPP ~odFile();
};
