#pragma once

#include <od/engine/module.h>

#define OD_LUA_NAMESPACE "odClientWrapper"
#define OD_LUA_METATABLE_NAME_KEY "_metatable_name"
#define OD_LUA_CPP_TYPE_KEY "_cpp_type"
#define OD_LUA_DEFAULT_NEW_KEY "new_default"

#define OD_LUA_STACK_TOP -1

struct lua_State;

struct odType;

typedef int(odLuaFn)(struct lua_State* /*state*/);

struct odLuaBinding {
	const char* name;
	odLuaFn *fn;
};

OD_API_C OD_ENGINE_MODULE const char*
odLua_get_error(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE int32_t
odLua_get_length(struct lua_State* lua, int32_t index);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD void*
odLua_get_userdata(struct lua_State* lua, int32_t index);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD void*
odLua_get_userdata_typed(struct lua_State* lua, int32_t index, const char* metatable_name);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_metatable_declare(struct lua_State* lua, const char* metatable_name);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_metatable_set_double(struct lua_State* lua, const char* metatable_name, const char* name, double value);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_metatable_set_string(struct lua_State* lua, const char* metatable_name, const char* name, char const* value);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_metatable_set_function(struct lua_State* lua, const char* metatable_name, const char* name, odLuaFn *fn);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_metatable_set_ptr(struct lua_State* lua, const char* metatable_name, const char* name, void* ptr);
// "type" instance must outlive "lua" instance
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_metatable_set_new_delete(struct lua_State* lua, const char* metatable_name, const struct odType* type);

OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_run_file(struct lua_State* lua, const char* filename, const char** args, int32_t args_count);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_run_string(struct lua_State* lua, const char* string, const char** args, int32_t args_count);
OD_API_C OD_ENGINE_MODULE bool
odLua_run_assert(struct lua_State* lua, const char* format_c_str, ...);
OD_API_C OD_ENGINE_MODULE OD_NO_DISCARD bool
odLua_run_check(struct lua_State* lua, const char* format_c_str, ...);
OD_API_C OD_ENGINE_MODULE void
odLua_print_stack(struct lua_State* lua);
