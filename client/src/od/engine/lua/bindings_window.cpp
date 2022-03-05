#include <od/engine/lua/bindings.h>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/platform/window.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

static bool odLuaBindings_odWindow_get_settings_impl(lua_State* lua, odWindowSettings* settings, int32_t settings_index) {
	if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(settings != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return false;
	}

	lua_getfield(lua, settings_index, "width");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TNUMBER)) {
			return luaL_error(lua, "settings.width must be a number or nil");
		}

		settings->width = static_cast<int32_t>(lua_tonumber(lua, OD_LUA_STACK_TOP));

		if (!OD_CHECK(settings->width > 0)) {
			return luaL_error(lua, "settings.width must be > 0");
		}
	}

	lua_getfield(lua, settings_index, "height");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TNUMBER)) {
			return luaL_error(lua, "settings.height must be a number or nil");
		}

		settings->height = static_cast<int32_t>(lua_tonumber(lua, OD_LUA_STACK_TOP));

		if (!OD_CHECK(settings->height > 0)) {
			return luaL_error(lua, "settings.width must be > 0");
		}
	}

	lua_getfield(lua, settings_index, "fps_limit");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TNUMBER)) {
			return luaL_error(lua, "settings.fps_limit must be a number or nil");
		}

		settings->fps_limit = static_cast<int32_t>(lua_tonumber(lua, OD_LUA_STACK_TOP));

		if (!OD_CHECK(settings->fps_limit > 0)) {
			return luaL_error(lua, "settings.fps_limit must be > 0");
		}
	}

	lua_getfield(lua, settings_index, "is_fps_limit_enabled");
	if ((lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL)) {
		if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TBOOLEAN)) {
			return luaL_error(lua, "settings.is_fps_limit_enabled must be a boolean or nil");
		}

		settings->is_fps_limit_enabled = static_cast<bool>(lua_toboolean(lua, OD_LUA_STACK_TOP));
	}

	lua_getfield(lua, settings_index, "is_vsync_enabled");
	if ((lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL)) {
		if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TBOOLEAN)) {
			return luaL_error(lua, "settings.is_fps_limit_enabled must be a boolean or nil");
		}

		settings->is_vsync_enabled = static_cast<bool>(lua_toboolean(lua, OD_LUA_STACK_TOP));
	}

	lua_getfield(lua, settings_index, "is_visible");
	if ((lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL)) {
		if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TBOOLEAN)) {
			return luaL_error(lua, "settings.is_visible must be a boolean or nil");
		}

		settings->is_visible = static_cast<bool>(lua_toboolean(lua, OD_LUA_STACK_TOP));
	}

	return true;
}
static int odLuaBindings_odWindow_init(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_WINDOW));
	if (!OD_CHECK(odWindow_check_valid(window))) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_WINDOW);
	}

	odWindowSettings settings{*odWindowSettings_get_defaults()};

	if (!OD_CHECK(odLuaBindings_odWindow_get_settings_impl(lua, &settings, settings_index))) {
		return luaL_error(lua, "odLuaBindings_odWindow_get_settings_impl() failed");
	}

	if (!OD_CHECK(odWindow_init(window, &settings))) {
		return luaL_error(lua, "odWindow_init() failed, settings=%s", odWindowSettings_get_debug_string(&settings));
	}

	return 0;
}
static int odLuaBindings_odWindow_set_settings(lua_State* lua) {
	const int self_index = 1;
	const int settings_index = 2;

	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_WINDOW));
	if (!OD_CHECK(odWindow_check_valid(window))) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_WINDOW);
	}

	odWindowSettings settings{*odWindow_get_settings(window)};

	if (!OD_CHECK(odLuaBindings_odWindow_get_settings_impl(lua, &settings, settings_index))) {
		return luaL_error(lua, "odLuaBindings_odWindow_get_settings_impl() failed");
	}

	if (!OD_CHECK(odWindow_set_settings(window, &settings))) {
		return luaL_error(
			lua,
			"odWindow_set_settings() failed, settings=%s, window=%s",
			odWindowSettings_get_debug_string(&settings),
			odWindow_get_debug_string(window)
		);
	}

	return 0;
}
static int odLuaBindings_odWindow_get_settings(lua_State* lua) {
	const int self_index = 1;

	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_WINDOW));
	if (!OD_CHECK(odWindow_check_valid(window))) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_WINDOW);
	}

	const odWindowSettings* settings = odWindow_get_settings(window);
	if (!OD_CHECK(settings != nullptr)) {
		return luaL_error(lua, "odWindow_get_settings() failed");
	}

	lua_newtable(lua);
	const int settings_index = lua_gettop(lua);

	lua_pushnumber(lua, static_cast<lua_Number>(settings->width));
	lua_setfield(lua, settings_index, "width");
	lua_pushnumber(lua, static_cast<lua_Number>(settings->height));
	lua_setfield(lua, settings_index, "height");
	lua_pushnumber(lua, static_cast<lua_Number>(settings->fps_limit));
	lua_setfield(lua, settings_index, "fps_limit");
	lua_pushboolean(lua, settings->is_fps_limit_enabled);
	lua_setfield(lua, settings_index, "is_fps_limit_enabled");
	lua_pushboolean(lua, settings->is_vsync_enabled);
	lua_setfield(lua, settings_index, "is_vsync_enabled");
	lua_pushboolean(lua, settings->is_visible);
	lua_setfield(lua, settings_index, "is_visible");

	lua_pushvalue(lua, settings_index);
	return 1;
}
static int odLuaBindings_odWindow_destroy(lua_State* lua) {
	const int self_index = 1;

	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_WINDOW));
	if (!OD_CHECK(window != nullptr)) {
		return 0;
	}

	odWindow_destroy(window);

	return 0;
}
static int odLuaBindings_odWindow_step(lua_State* lua) {
	const int self_index = 1;

	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_WINDOW));
	if (!OD_CHECK(odWindow_check_valid(window))) {
		return luaL_error(lua, "odWindow_check_valid() failed");
	}

	lua_pushboolean(lua, odWindow_step(window));
	return 1;
}
bool odLuaBindings_odWindow_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_declare(lua, OD_LUA_BINDINGS_WINDOW))
		|| !OD_CHECK(odLua_metatable_set_new_delete(lua, OD_LUA_BINDINGS_WINDOW, odType_get<odWindow>()))) {
		return false;
	}

	auto add_method = [lua](const char* name, odLuaFn* fn) -> bool {
		return odLua_metatable_set_function(lua, OD_LUA_BINDINGS_WINDOW, name, fn);
	};
	if (!OD_CHECK(add_method("init", odLuaBindings_odWindow_init))
		|| !OD_CHECK(add_method("destroy", odLuaBindings_odWindow_destroy))
		|| !OD_CHECK(add_method("step", odLuaBindings_odWindow_step))
		|| !OD_CHECK(add_method("set_settings", odLuaBindings_odWindow_set_settings))
		|| !OD_CHECK(add_method("get_settings", odLuaBindings_odWindow_get_settings))) {
		return false;
	}

	return true;
}
