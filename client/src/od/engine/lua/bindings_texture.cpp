#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/platform/file.h>
#include <od/platform/image.hpp>
#include <od/platform/texture.hpp>
#include <od/platform/window.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

static int odLuaBindings_odTexture_init(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odTexture* texture = static_cast<odTexture*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_TEXTURE));

	if (!OD_CHECK(texture != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_TEXTURE);
	}

	lua_getfield(lua, settings_index, "window");
	const int window_index = lua_gettop(lua);
	odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(lua, window_index, OD_LUA_BINDINGS_WINDOW));

	if (!OD_CHECK(odWindow_check_valid(window))) {
		return luaL_error(lua, "odWindow_check_valid() failed");
	}

	lua_getfield(lua, settings_index, "width");
	int32_t width = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP));

	if (!OD_CHECK(width > 0)) {
		return luaL_argerror(lua, settings_index, "settings.width must be > 0");
	}

	lua_getfield(lua, settings_index, "height");
	int32_t height = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP));

	if (!OD_CHECK(height > 0)) {
		return luaL_argerror(lua, settings_index, "settings.height must be > 0");
	}

	odImage image{};
	if (!OD_CHECK(odTexture_init(texture, window, nullptr, width, height))) {
		return luaL_error(lua, "odTexture_init() failed, width=%d, height=%d", width, height);
	}

	return 0;
}
static int odLuaBindings_odTexture_init_from_png_file(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odTexture* texture = static_cast<odTexture*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_TEXTURE));

	if (!OD_CHECK(texture != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_TEXTURE);
	}

	lua_getfield(lua, settings_index, "window");
	const int window_index = lua_gettop(lua);
	odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(lua, window_index, OD_LUA_BINDINGS_WINDOW));
	if (!OD_CHECK(odWindow_check_valid(window))) {
		return luaL_error(lua, "odWindow_check_valid() failed");
	}

	const char* filename = "";
	lua_getfield(lua, settings_index, "filename");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		filename = luaL_checkstring(lua, OD_LUA_STACK_TOP);

		if (!OD_CHECK(odFile_get_exists(filename))) {
			OD_ERROR("Nonexistent filename=%s", filename);
			return luaL_argerror(lua, settings_index, "filename doesn't exist");
		}
	}

	odImage image{};
	if (!OD_CHECK(odImage_read_png_file(&image, filename))) {
		return luaL_error(lua, "odImage_read_png_file() filename=%s", filename);
	}

	if (!OD_CHECK(odTexture_init(texture, window, odImage_begin_const(&image), image.width, image.height))) {
		return luaL_error(lua, "odTexture_init() failed, width=%d, height=%d", image.width, image.height);
	}

	return 0;
}
static int odLuaBindings_odTexture_new(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int settings_index = 1;

	luaL_checktype(lua, settings_index, LUA_TTABLE);

	const int32_t metatable_index = lua_upvalueindex(1);

	luaL_checktype(lua, metatable_index, LUA_TTABLE);

	lua_getfield(lua, metatable_index, OD_LUA_DEFAULT_NEW_KEY);
	if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TFUNCTION)) {
		return luaL_error(lua, "metatable.%s must be of type function", OD_LUA_DEFAULT_NEW_KEY);
	}

	lua_call(lua, /*nargs*/ 0, /*nresults*/ 1);  // call metatable.default_new
	const int self_index = lua_gettop(lua);

	lua_getfield(lua, self_index, "init");
	if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TFUNCTION)) {
		return luaL_error(lua, "metatable.init must be of type function");
	}

	lua_pushvalue(lua, self_index);
	lua_pushvalue(lua, settings_index);
	lua_call(lua, /*nargs*/ 2, /*nresults*/ 1);

	lua_pushvalue(lua, self_index);
	return 1;
}
static int odLuaBindings_odTexture_new_from_png_file(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int settings_index = 1;

	luaL_checktype(lua, settings_index, LUA_TTABLE);

	const int32_t metatable_index = lua_upvalueindex(1);

	luaL_checktype(lua, metatable_index, LUA_TTABLE);

	lua_getfield(lua, metatable_index, OD_LUA_DEFAULT_NEW_KEY);
	if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TFUNCTION)) {
		return luaL_error(lua, "metatable.%s must be of type function", OD_LUA_DEFAULT_NEW_KEY);
	}

	lua_call(lua, /*nargs*/ 0, /*nresults*/ 1);  // call metatable.default_new
	const int self_index = lua_gettop(lua);

	lua_getfield(lua, self_index, "init_from_png_file");
	if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TFUNCTION)) {
		return luaL_error(lua, "metatable.init must be of type function");
	}

	lua_pushvalue(lua, self_index);
	lua_pushvalue(lua, settings_index);
	lua_call(lua, /*nargs*/ 2, /*nresults*/ 1);

	lua_pushvalue(lua, self_index);
	return 1;
}
static int odLuaBindings_odTexture_destroy(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	odTexture* texture = static_cast<odTexture*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_TEXTURE));
	if (!OD_CHECK(texture != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_TEXTURE);
	}

	odTexture_destroy(texture);

	return 0;
}
static int odLuaBindings_odTexture_get_size(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const odTexture* texture = static_cast<odTexture*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_TEXTURE));
	if (!OD_CHECK(texture != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_TEXTURE);
	}

	int32_t width = 0;
	int32_t height = 0;
	if (!OD_CHECK(odTexture_get_size(texture, &width, &height))) {
		return 0;
	}

	lua_pushnumber(lua, static_cast<lua_Number>(width));
	lua_pushnumber(lua, static_cast<lua_Number>(height));
	return 2;
}
bool odLuaBindings_odTexture_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_declare(lua, OD_LUA_BINDINGS_TEXTURE))
		|| !OD_CHECK(odLua_metatable_set_new_delete(lua, OD_LUA_BINDINGS_TEXTURE, odType_get<odTexture>()))) {
		return false;
	}

	auto add_method = [lua](const char* name, odLuaFn* fn) -> bool {
		return odLua_metatable_set_function(lua, OD_LUA_BINDINGS_TEXTURE, name, fn);
	};
	if (!OD_CHECK(add_method("init", odLuaBindings_odTexture_init))
		|| !OD_CHECK(add_method("init_from_png_file", odLuaBindings_odTexture_init_from_png_file))
		|| !OD_CHECK(add_method("new", odLuaBindings_odTexture_new))
		|| !OD_CHECK(add_method("new_from_png_file", odLuaBindings_odTexture_new_from_png_file))
		|| !OD_CHECK(add_method("destroy", odLuaBindings_odTexture_destroy))
		|| !OD_CHECK(add_method("get_size", odLuaBindings_odTexture_get_size))) {
		return false;
	}

	return true;
}
