#pragma once

#include <od/engine/module.h>

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEngine_init(struct odEngine* engine);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEngine_step(struct odEngine* engine);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odEngine_run(struct odEngine* engine);
