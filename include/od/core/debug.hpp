#pragma once

#include <od/core/debug.h>

struct odLogLevelScoped {
	uint32_t backup_log_level;

	OD_ENGINE_CORE_MODULE odLogLevelScoped();
	OD_ENGINE_CORE_MODULE explicit odLogLevelScoped(uint32_t log_level);
	OD_ENGINE_CORE_MODULE ~odLogLevelScoped();
};
