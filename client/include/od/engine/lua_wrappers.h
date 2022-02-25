#pragma once

#include <od/engine/module.h>

#define OD_LUA_STACK_TOP -1

struct lua_State;

struct odType;

struct odLuaBinding {
	const char* name;
	int (*fn)(struct lua_State* state);
};

OD_API_C OD_ENGINE_MODULE const char*
odLua_get_error(struct lua_State* lua);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD void*
odLua_function_get_userdata(struct lua_State* lua);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_metatable_declare(struct lua_State* lua, const char* metatable_name);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_metatable_set_double(struct lua_State* lua, const char* metatable_name, const char* name, double value);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_metatable_set_function(struct lua_State* lua, const char* metatable_name, const char* name, int (*fn)(struct lua_State*));
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_metatable_set_ptr(struct lua_State* lua, const char* metatable_name, const char* name, void* ptr);
// "type" instance must outlive "lua" instance
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_metatable_set_new_delete(struct lua_State* lua, const char* metatable_name, const struct odType* type);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_run_file(struct lua_State* lua, const char* file_path, const char** args, int32_t args_count);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_run_string(struct lua_State* lua, const char* string, const char** args, int32_t args_count);
OD_API_C OD_ENGINE_MODULE void
odLua_print_stack(struct lua_State* lua);
