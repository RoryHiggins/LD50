#pragma once

#include <od/engine/lua_wrappers.h>

struct odLuaScope {
	lua_State* lua;
	int32_t stack_pos;

	OD_ENGINE_MODULE void return_count(int32_t count);

	OD_ENGINE_MODULE explicit odLuaScope(lua_State* in_lua);
	OD_ENGINE_MODULE ~odLuaScope();

	odLuaScope(const odLuaScope& other) = delete;
	OD_ENGINE_MODULE odLuaScope(odLuaScope&& other) = delete;
	odLuaScope& operator=(const odLuaScope& other) = delete;
	OD_ENGINE_MODULE odLuaScope& operator=(odLuaScope&& other)  = delete;
};
