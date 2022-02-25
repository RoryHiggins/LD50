#pragma once

#include <od/engine/module.h>

#include <od/platform/window.h>

struct lua_State;

struct odLuaClient;

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLuaClient_init(struct odLuaClient* client);
OD_API_C OD_ENGINE_MODULE void
odLuaClient_destroy(struct odLuaClient* client);
OD_API_C OD_PLATFORM_MODULE void
odLuaClient_swap(struct odLuaClient* client1, struct odLuaClient* client2);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odLuaClient_check_valid(const struct odLuaClient* client);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD struct lua_State*
odLuaClient_get_lua(struct odLuaClient* client);
