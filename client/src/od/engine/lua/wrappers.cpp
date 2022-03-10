#include <od/engine/lua/wrappers.hpp>

#include <cstdlib>
#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/string.hpp>
#include <od/engine/lua/includes.h>

static const odType* odLua_table_get_type(lua_State* lua, int32_t metatable_index) {
	if (!OD_CHECK(lua != nullptr)) {
		return nullptr;
	}

	odLuaScope scope{lua};

	lua_getfield(lua, metatable_index, OD_LUA_CPP_TYPE_KEY);
	if (!OD_CHECK(lua_islightuserdata(lua, OD_LUA_STACK_TOP))) {
		return nullptr;
	}

	return static_cast<odType*>(lua_touserdata(lua, OD_LUA_STACK_TOP));
}
static int odLua_cpp_new(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int32_t metatable_index = lua_upvalueindex(1);

	luaL_checktype(lua, metatable_index, LUA_TTABLE);

	const odType* type = odLua_table_get_type(lua, metatable_index);
	if (!OD_CHECK(type != nullptr)) {
		return luaL_error(lua, "odLua_table_get_type() failed");
	}

	size_t ptr_size = sizeof(void*);
	void** userdata = static_cast<void**>(lua_newuserdata(lua, ptr_size));
	if (!OD_CHECK(userdata != nullptr)) {
		return luaL_error(lua, "userdata creation failed for size=%llu", ptr_size);
	}

	size_t allocation_size = static_cast<size_t>(type->size);
	void* allocation = calloc(1, allocation_size);
	if (!OD_CHECK(allocation != nullptr)) {
		return luaL_error(lua, "calloc() failed for size=%llu", allocation_size);
	}

	lua_pushvalue(lua, metatable_index);
	lua_setmetatable(lua, OD_LUA_STACK_TOP - 1);

	// construct object
	type->default_construct_fn(allocation, 1);
	*userdata = allocation;

	return 1;
}
static int odLua_cpp_delete(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int32_t userdata_index = 1;

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
	
	// not a check as it is ok for allocation to already be unset (never initted, or already deleted)
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
int32_t odLua_get_length(struct lua_State* lua, int32_t index) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

#if LUA_VERSION_NUM < 502
	return static_cast<int32_t>(lua_objlen(lua, index));
#else
	return static_cast<int32_t>(lua_rawlen(lua, index));
#endif
}
void* odLua_get_userdata(lua_State* lua, int32_t index) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return nullptr;
	}

	odLuaScope scope{lua};

	if (!OD_DEBUG_CHECK(lua_type(lua, index) == LUA_TUSERDATA)) {
		return nullptr;
	}

	void** userdata = static_cast<void**>(lua_touserdata(lua, index));
	if (!OD_CHECK(userdata != nullptr)) {
		OD_ERROR(
			"lua_touserdata(index=%d) returned nullptr; "
			"possibly a method called with '.' instead of ':?",
			index);
		return nullptr;
	}

	void* allocation = *userdata;
	if (!OD_CHECK(allocation != nullptr)) {
		return nullptr;
	}

	return allocation;
}
void* odLua_get_userdata_typed(lua_State* lua, int32_t index, const char* metatable_name) {
	if (!OD_CHECK(lua != nullptr)) {
		return nullptr;
	}

	odLuaScope scope{lua};

	if (OD_BUILD_DEBUG) {
		lua_getfield(lua, index, OD_LUA_METATABLE_NAME_KEY);
		if (!OD_DEBUG_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TSTRING)) {
			return nullptr;
		}

		const char* actual_metatable_name = lua_tostring(lua, OD_LUA_STACK_TOP);
		if (!OD_DEBUG_CHECK(actual_metatable_name != nullptr)
			|| !OD_DEBUG_CHECK(strcmp(actual_metatable_name, metatable_name) == 0)) {
			return nullptr;
		}
	}

	return odLua_get_userdata(lua, index);
}
bool odLua_metatable_declare(lua_State* lua, const char* metatable_name) {
	if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(metatable_name != nullptr)) {
		return false;
	}

	odLuaScope scope{lua};

	// global scope metatable
	lua_getglobal(lua, OD_LUA_NAMESPACE);
	if (!lua_istable(lua, OD_LUA_STACK_TOP)) {
		lua_newtable(lua);

		lua_pushvalue(lua, OD_LUA_STACK_TOP);
		lua_setglobal(lua, OD_LUA_NAMESPACE);
	}
	int namespace_index = lua_gettop(lua);

	// 0 = already exists, 1 = new, need to set index and assign to global
	lua_getfield(lua, OD_LUA_STACK_TOP, metatable_name);
	if (!lua_istable(lua, OD_LUA_STACK_TOP)) {
		lua_newtable(lua);

		lua_pushvalue(lua, OD_LUA_STACK_TOP);
		lua_setfield(lua, OD_LUA_STACK_TOP, "__index");

		lua_pushvalue(lua, OD_LUA_STACK_TOP);
		lua_setfield(lua, namespace_index, metatable_name);

		lua_pushstring(lua, OD_LUA_METATABLE_NAME_KEY);
		lua_pushstring(lua, metatable_name);
		lua_settable(lua, OD_LUA_STACK_TOP - 2);
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

	lua_getglobal(lua, OD_LUA_NAMESPACE);
	lua_getfield(lua, OD_LUA_STACK_TOP, metatable_name);

	lua_pushstring(lua, name);
	lua_pushnumber(lua, value);
	lua_settable(lua, OD_LUA_STACK_TOP - 2);

	return true;
}
bool odLua_metatable_set_string(lua_State* lua, const char* metatable_name, const char* name, char const* value) {
		if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(metatable_name != nullptr)
		|| !OD_CHECK(name != nullptr)) {
		return false;
	}

	odLuaScope scope{lua};

	lua_getglobal(lua, OD_LUA_NAMESPACE);
	lua_getfield(lua, OD_LUA_STACK_TOP, metatable_name);

	lua_pushstring(lua, name);
	lua_pushstring(lua, value);
	lua_settable(lua, OD_LUA_STACK_TOP - 2);

	return true;
}
bool odLua_metatable_set_function(lua_State* lua, const char* metatable_name, const char* name, odLuaFn *fn) {
	if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(metatable_name != nullptr)
		|| !OD_CHECK(name != nullptr)
		|| !OD_CHECK(fn != nullptr)) {
		return false;
	}

	odLuaScope scope{lua};

	lua_getglobal(lua, OD_LUA_NAMESPACE);
	lua_getfield(lua, OD_LUA_STACK_TOP, metatable_name);

	lua_pushstring(lua, name);
	lua_pushvalue(lua, OD_LUA_STACK_TOP - 1);  // _G[namespace][metatable_name]
	lua_pushcclosure(lua, fn, 1);  // metatable as the sole upvalue
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

	lua_getglobal(lua, OD_LUA_NAMESPACE);
	lua_getfield(lua, OD_LUA_STACK_TOP, metatable_name);

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

	void* type_raw = static_cast<void*>(const_cast<odType*>(type));

	// these should never get overridden by bindings
	if (!OD_CHECK(odLua_metatable_set_ptr(lua, metatable_name, OD_LUA_CPP_TYPE_KEY, type_raw))) {
		return false;
	}
	if (!OD_CHECK(odLua_metatable_set_function(lua, metatable_name, OD_LUA_DEFAULT_NEW_KEY, odLua_cpp_new))) {
		return false;
	}
	if (!OD_CHECK(odLua_metatable_set_function(lua, metatable_name, "__gc", odLua_cpp_delete))) {
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

	int run_result = lua_pcall(lua, args_count, /*nresults*/ LUA_MULTRET, /*errFunc*/ 0);
	if (run_result != 0) {
		OD_INFO(
			"lua_pcall() failed, filename=%s, result=%d, error=%s",
			filename,
			run_result,
			odLua_get_error(lua)
		);
		return false;
	}

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

	int run_result = lua_pcall(lua, args_count, /*nresults*/ LUA_MULTRET, /*errFunc*/ 0);
	if (run_result != 0) {
		OD_ERROR(
			"lua_pcall() failed, string=%s, result=%d, error=%s",
			string,
			run_result,
			odLua_get_error(lua)
		);
		return false;
	}

	return true;
}
bool odLua_run_check(lua_State* lua, const char* format_c_str, ...) {
	if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(format_c_str != nullptr)) {
		return false;
	}

	odString str;
	if (!OD_CHECK(str.extend("assert("))) {
		return false;
	}

	va_list args = {};
	va_start(args, format_c_str);
	bool extend_formatted_variadic_ok = str.extend_formatted_variadic(format_c_str, &args);
	va_end(args);

	if (!OD_CHECK(extend_formatted_variadic_ok)) {
		return false;
	}

	if (!OD_CHECK(str.extend(")"))) {
		return false;
	}

	return OD_CHECK(odLua_run_string(lua, str.get_c_str(), nullptr, 0));
}
bool odLua_run_assert(lua_State* lua, const char* format_c_str, ...) {
	if (!OD_ASSERT(lua != nullptr)
		|| !OD_ASSERT(format_c_str != nullptr)) {
		return false;
	}

	odString str;
	if (!OD_ASSERT(str.extend("assert("))) {
		return false;
	}

	va_list args = {};
	va_start(args, format_c_str);
	bool extend_formatted_variadic_ok = str.extend_formatted_variadic(format_c_str, &args);
	va_end(args);

	if (!OD_ASSERT(extend_formatted_variadic_ok)) {
		return false;
	}

	if (!OD_ASSERT(str.extend(")"))) {
		return false;
	}

	return OD_ASSERT(odLua_run_string(lua, str.get_c_str(), nullptr, 0));
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

	fflush(stdout);
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
