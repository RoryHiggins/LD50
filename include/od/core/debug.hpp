#pragma once

#include <od/core/debug.h>

struct odLogLevelScoped {
	uint32_t backup_log_level;

	OD_API_ENGINE_CPP odLogLevelScoped();
	OD_API_ENGINE_CPP explicit odLogLevelScoped(uint32_t log_level);
	OD_API_ENGINE_CPP ~odLogLevelScoped();
};
