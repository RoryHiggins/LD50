#include <od/engine/lua_wrappers.hpp>

// #include <cstddef>
#include <cstdlib>

#include <od/core/debug.h>
#include <od/core/type.hpp>

#include <od/engine/lua_includes.h>

static const odType* odLua_table_get_type(lua_State* lua, int32_t metatable_index) {
	if (!OD_CHECK(lua != nullptr)) {
		return nullptr;
	}

	odLuaScope scope{lua};

	lua_pushvalue(lua, metatable_index);
	lua_getfield(lua, metatable_index, "_cpp_type");
	if (!OD_CHECK(lua_islightuserdata(lua, OD_LUA_STACK_TOP))) {
		return nullptr;
	}

	return static_cast<odType*>(lua_touserdata(lua, OD_LUA_STACK_TOP));
}
static int odLuaFn_table_new(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	odLuaScope scope{lua};

	int32_t metatable_index = lua_upvalueindex(1);

	const odType* type = odLua_table_get_type(lua, metatable_index);
	if (!OD_CHECK(type != nullptr)) {
		return 0;
	}

	void** userdata = static_cast<void**>(lua_newuserdata(lua, sizeof(void*)));
	if (!OD_CHECK(userdata != nullptr)) {
		return 0;
	}

	void* allocation = calloc(1, static_cast<size_t>(type->size));
	if (!OD_CHECK(allocation != nullptr)) {
		return 0;
	}

	lua_pushvalue(lua, metatable_index);
	lua_setmetatable(lua, OD_LUA_STACK_TOP - 1);

	// construct object
	type->default_construct_fn(allocation, 1);
	*userdata = allocation;

	scope.return_count(1);
	return 1;
}
static int odLuaFn_table_delete(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	odLuaScope scope{lua};

	int32_t userdata_index = 1;

	const odType* type = odLua_table_get_type(lua, userdata_index);
	if (!OD_CHECK(type != nullptr)) {
		return 0;
	}

	void** userdata = static_cast<void**>(lua_touserdata(lua, userdata_index));
	if (!OD_CHECK(userdata != nullptr)) {
		return 0;
	}

	void* allocation = *userdata;
	*userdata = nullptr; // clear userdata to prevent double free
	if (allocation != nullptr) {
		type->destruct_fn(allocation, 1);
		free(allocation);
	}

	return 0;
}

const char* odLua_get_error(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return "";
	}

	const char* error_str = lua_tostring(lua, OD_LUA_STACK_TOP);
	if (error_str == nullptr) {
		return "";
	}

	return error_str;
}
void* odLua_function_get_userdata(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return nullptr;
	}

	odLuaScope scope{lua};

	int32_t userdata_index = 1;

	void** userdata = static_cast<void**>(lua_touserdata(lua, userdata_index));
	if (userdata == nullptr) {
		luaL_argerror(lua, userdata_index, "userdata arg not provided; possibly a method called with '.' instead of ':'?");
		return nullptr;
	}

	void* allocation = *userdata;
	if (!OD_CHECK(allocation != nullptr)) {
		return nullptr;
	}

	return allocation;
}
bool odLua_metatable_declare(lua_State* lua, const char* metatable_name) {
	if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(metatable_name != nullptr)) {
		return false;
	}

	odLuaScope scope{lua};

	// 0 = already exists, 1 = new, need to set index and assign to global
	if (luaL_newmetatable(lua, metatable_name) == 1) {
		lua_pushvalue(lua, OD_LUA_STACK_TOP);
		lua_setfield(lua, OD_LUA_STACK_TOP, "__index");

		lua_pushvalue(lua, OD_LUA_STACK_TOP);
		lua_setglobal(lua, metatable_name);
	}

	return true;
}
bool odLua_metatable_set_double(lua_State* lua, const char* metatable_name, const char* name, double value) {
	if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(metatable_name != nullptr)
		|| !OD_CHECK(name != nullptr)) {
		return false;
	}

	odLuaScope scope{lua};

	luaL_getmetatable(lua, metatable_name);
	lua_pushstring(lua, name);
	lua_pushnumber(lua, value);

	lua_settable(lua, OD_LUA_STACK_TOP - 2);

	return true;
}
bool odLua_metatable_set_function(lua_State* lua, const char* metatable_name, const char* name, int (*fn)(lua_State*)) {
	if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(metatable_name != nullptr)
		|| !OD_CHECK(name != nullptr)
		|| !OD_CHECK(fn != nullptr)) {
		return false;
	}

	odLuaScope scope{lua};

	luaL_getmetatable(lua, metatable_name);

	lua_pushstring(lua, name);

	luaL_getmetatable(lua, metatable_name);
	lua_pushcclosure(lua, fn, 1);

	lua_settable(lua, OD_LUA_STACK_TOP - 2);

	return true;
}
bool odLua_metatable_set_ptr(lua_State* lua, const char* metatable_name, const char* name, void* ptr) {
	if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(metatable_name != nullptr)
		|| !OD_CHECK(name != nullptr)) {
		return false;
	}

	odLuaScope scope{lua};

	luaL_getmetatable(lua, metatable_name);
	lua_pushstring(lua, name);
	lua_pushlightuserdata(lua, ptr);

	lua_settable(lua, OD_LUA_STACK_TOP - 2);

	return true;
}
bool odLua_metatable_set_new_delete(lua_State* lua, const char* metatable_name, const odType* type) {
	if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(metatable_name != nullptr)
		|| !OD_CHECK(type != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_set_ptr(lua, metatable_name, "_cpp_type", static_cast<void*>(const_cast<odType*>(type))))) {
		return false;
	}
	if (!OD_CHECK(odLua_metatable_set_function(lua, metatable_name, "new", odLuaFn_table_new))) {
		return false;
	}
	if (!OD_CHECK(odLua_metatable_set_function(lua, metatable_name, "__gc", odLuaFn_table_delete))) {
		return false;
	}

	return true;
}

bool odLua_run_file(lua_State* lua, const char* filename, const char** args, int32_t args_count) {
	if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(filename != nullptr)
		|| !OD_CHECK((args != nullptr) || (args_count == 0))
		|| !OD_CHECK(args_count >= 0)) {
		return false;
	}

	OD_DEBUG("filename=%s", filename);

	int load_file_result = luaL_loadfile(lua, filename);
	if (!OD_CHECK(load_file_result == 0)) {
		OD_ERROR(
			"luaL_loadfile() failed, filename=%s, result=%d, error=%s",
			filename,
			load_file_result,
			odLua_get_error(lua)
		);
		return false;
	}

	for (int32_t i = 0; i < args_count; i++) {
		OD_TRACE("arg[%d] = \"%s\"", i, args[i]);
		lua_pushstring(lua, args[i]);
	}
	OD_TRACE("args_count=%d", args_count);

	int run_result = lua_pcall(lua, args_count, /*nresults*/ 0, /*errFunc*/ 0);
	if (!OD_CHECK(run_result == 0)) {
		OD_ERROR(
			"lua_pcall() failed, filename=%s, result=%d, error=%s",
			filename,
			run_result,
			odLua_get_error(lua)
		);
		return false;
	}

	lua_settop(lua, 0);

	return true;
}
bool odLua_run_string(lua_State* lua, const char* string, const char** args, int32_t args_count) {
		if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(string != nullptr)
		|| !OD_CHECK((args != nullptr) || (args_count == 0))
		|| !OD_CHECK(args_count >= 0)) {
		return false;
	}

	OD_DEBUG("string=%4096s", string);

	int load_string_result = luaL_loadstring(lua, string);
	if (!OD_CHECK(load_string_result == 0)) {
		OD_ERROR(
			"luaL_loadstring() failed, string=%s, result=%d, error=%s",
			string,
			load_string_result,
			odLua_get_error(lua)
		);
		return false;
	}

	for (int32_t i = 0; i < args_count; i++) {
		OD_TRACE("arg[%d] = \"%s\"", i, args[i]);
		lua_pushstring(lua, args[i]);
	}
	OD_TRACE("args_count=%d", args_count);

	int run_result = lua_pcall(lua, args_count, /*nresults*/ 0, /*errFunc*/ 0);
	if (run_result != 0) {
		OD_ERROR(
			"lua_pcall() failed, string=%s, result=%d, error=%s",
			string,
			run_result,
			odLua_get_error(lua)
		);
		return false;
	}

	lua_settop(lua, 0);

	return true;
}
void odLua_print_stack(lua_State* lua) {
	odLuaScope scope{lua};

	int top = lua_gettop(lua);
	puts("--- lua stack dump ---");
	for (int32_t i = 0; i <= top; i++) {
		printf("[%d] : %s = ", i, luaL_typename(lua, i));

		switch (lua_type(lua, i)) {
			case LUA_TNIL: {
				printf("%s\n", "nil");
				break;
			}
			case LUA_TBOOLEAN: {
				printf("%s\n", lua_toboolean(lua, i) ? "true" : "false");
				break;
			}
			case LUA_TNUMBER:{
				printf("%g\n", lua_tonumber(lua, i));
				break;
			}
			case LUA_TSTRING:{
				printf("\"%s\"\n", lua_tostring(lua, i));
				break;
			}
			default: {
				printf("%p\n", lua_topointer(lua, i));
				break;
			}
		}
	}
}

void odLuaScope::return_count(int32_t count) {
	stack_pos -= count;
}
odLuaScope::odLuaScope(lua_State* in_lua)
: lua{in_lua}, stack_pos{lua_gettop(in_lua)} {
}
odLuaScope::~odLuaScope() {
	if (lua != nullptr) {
		lua_settop(lua, stack_pos);
	}

	lua = nullptr;
	stack_pos = 0;
}
