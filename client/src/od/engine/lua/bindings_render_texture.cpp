#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/platform/file.h>
#include <od/platform/image.hpp>
#include <od/platform/texture.hpp>
#include <od/platform/render_texture.hpp>
#include <od/platform/window.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

static int odLuaBindings_odRenderTexture_init(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odRenderTexture* render_texture = static_cast<odRenderTexture*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_RENDER_TEXTURE));

	if (!OD_CHECK(render_texture != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDER_TEXTURE);
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
	if (!OD_CHECK(odRenderTexture_init(render_texture, window, width, height))) {
		return luaL_error(lua, "odRenderTexture_init() failed, width=%d, height=%d", width, height);
	}

	return 0;
}
static int odLuaBindings_odRenderTexture_new(lua_State* lua) {
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
static int odLuaBindings_odRenderTexture_destroy(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	odRenderTexture* render_texture = static_cast<odRenderTexture*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_RENDER_TEXTURE));
	if (!OD_CHECK(render_texture != nullptr)) {
		return 0;
	}

	odRenderTexture_destroy(render_texture);

	return 0;
}
static int odLuaBindings_odRenderTexture_get_size(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const odRenderTexture* render_texture = static_cast<odRenderTexture*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_RENDER_TEXTURE));
	if (!OD_CHECK(render_texture != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDER_TEXTURE);
	}

	const odTexture* texture = odRenderTexture_get_texture_const(render_texture);
	if (!OD_CHECK(texture != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDER_TEXTURE);
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
bool odLuaBindings_odRenderTexture_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_declare(lua, OD_LUA_BINDINGS_RENDER_TEXTURE))
		|| !OD_CHECK(odLua_metatable_set_new_delete(lua, OD_LUA_BINDINGS_RENDER_TEXTURE, odType_get<odRenderTexture>()))) {
		return false;
	}

	auto add_method = [lua](const char* name, odLuaFn* fn) -> bool {
		return odLua_metatable_set_function(lua, OD_LUA_BINDINGS_RENDER_TEXTURE, name, fn);
	};
	if (!OD_CHECK(add_method("init", odLuaBindings_odRenderTexture_init))
		|| !OD_CHECK(add_method("new", odLuaBindings_odRenderTexture_new))
		|| !OD_CHECK(add_method("destroy", odLuaBindings_odRenderTexture_destroy))
		|| !OD_CHECK(add_method("get_size", odLuaBindings_odRenderTexture_get_size))) {
		return false;
	}

	return true;
}
