#pragma once

#include <od/engine/module.h>

#include <od/platform/window.h>

struct odEngine;

struct odEngineSettings {
	struct odWindowSettings window;

	int32_t game_width;
	int32_t game_height;
};

OD_API_C OD_PLATFORM_MODULE const struct odEngineSettings*
odEngineSettings_get_defaults(void);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEngine_init(struct odEngine* engine, const struct odEngineSettings* opt_settings);
OD_API_C OD_ENGINE_MODULE void
odEngine_destroy(struct odEngine* engine);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEngine_set_settings(struct odEngine* engine, const struct odEngineSettings* settings);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEngine_step(struct odEngine* engine);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEngine_run(struct odEngine* engine, const struct odEngineSettings* opt_settings);
