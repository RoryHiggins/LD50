#pragma once

#include <od/engine/lua/client.h>

struct odLuaClient {
	struct lua_State* lua;

	OD_ENGINE_MODULE odLuaClient();
	OD_ENGINE_MODULE odLuaClient(odLuaClient&& other);
	OD_ENGINE_MODULE odLuaClient& operator=(odLuaClient&& other);
	OD_ENGINE_MODULE ~odLuaClient();

	odLuaClient(const odLuaClient& other) = delete;
	odLuaClient& operator=(const odLuaClient& other) = delete;
};
