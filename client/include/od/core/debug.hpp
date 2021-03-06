#pragma once

#include <od/core/debug.h>

struct odLogLevelScoped {
	int32_t backup_log_level;

	OD_CORE_MODULE odLogLevelScoped();
	OD_CORE_MODULE explicit odLogLevelScoped(int32_t log_level);
	OD_CORE_MODULE ~odLogLevelScoped();

	odLogLevelScoped(const odLogLevelScoped&) = delete;
	odLogLevelScoped(odLogLevelScoped&&) = delete;
	odLogLevelScoped& operator=(const odLogLevelScoped&) = delete;
	odLogLevelScoped& operator=(odLogLevelScoped&&) = delete;
};
