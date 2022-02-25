#pragma once

#include <od/engine/module.h>

#include <od/platform/window.h>

struct odClient;

struct odClientSettings {
	struct odWindowSettings window;

	int32_t game_width;
	int32_t game_height;
};

OD_API_C OD_ENGINE_MODULE const struct odClientSettings*
odClientSettings_get_defaults(void);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odClientSettings_check_valid(const struct odClientSettings* settings);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odClient_init(struct odClient* client, const struct odClientSettings* opt_settings);
OD_API_C OD_ENGINE_MODULE void
odClient_destroy(struct odClient* client);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odClient_set_settings(struct odClient* client, const struct odClientSettings* settings);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odClient_step(struct odClient* client);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odClient_run(struct odClient* client, const struct odClientSettings* opt_settings);
