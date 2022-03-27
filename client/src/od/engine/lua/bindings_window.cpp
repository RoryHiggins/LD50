#include <od/engine/lua/bindings.h>

#include <cstdio>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/platform/window.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

// https://github.com/libsdl-org/SDL-historical-archive/blob/40dce8c788cb42a05d4569cd8df7e92d4ebb8976/src/events/SDL_keyboard.c#L287
static const char *odLuaBindings_odWindow_key_names[] = {
	"#",
	"'",
	",",
	"-",
	".",
	"/",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	";",
	"=",
	"[",
	"\\",
	"]",
	"`",
	"a",
	"ac back",
	"ac bookmarks",
	"ac forward",
	"ac home",
	"ac refresh",
	"ac search",
	"ac stop",
	"again",
	"alterase",
	"app1",
	"app2",
	"application",
	"audiofastforward",
	"audiomute",
	"audionext",
	"audioplay",
	"audioprev",
	"audiorewind",
	"audiostop",
	"b",
	"backspace",
	"brightnessdown",
	"brightnessup",
	"c",
	"calculator",
	"cancel",
	"capslock",
	"clear / again",
	"clear",
	"computer",
	"copy",
	"crsel",
	"currencysubunit",
	"currencyunit",
	"cut",
	"d",
	"decimalseparator",
	"delete",
	"displayswitch",
	"down",
	"e",
	"eject",
	"end",
	"escape",
	"execute",
	"exsel",
	"f",
	"f1",
	"f10",
	"f11",
	"f12",
	"f13",
	"f14",
	"f15",
	"f16",
	"f17",
	"f18",
	"f19",
	"f2",
	"f20",
	"f21",
	"f22",
	"f23",
	"f24",
	"f3",
	"f4",
	"f5",
	"f6",
	"f7",
	"f8",
	"f9",
	"find",
	"g",
	"h",
	"help",
	"home",
	"i",
	"insert",
	"j",
	"k",
	"kbdillumdown",
	"kbdillumtoggle",
	"kbdillumup",
	"keypad !",
	"keypad #",
	"keypad %",
	"keypad &",
	"keypad &&",
	"keypad (",
	"keypad )",
	"keypad *",
	"keypad +",
	"keypad +/-",
	"keypad ,",
	"keypad -",
	"keypad .",
	"keypad /",
	"keypad 0",
	"keypad 00",
	"keypad 000",
	"keypad 1",
	"keypad 2",
	"keypad 3",
	"keypad 4",
	"keypad 5",
	"keypad 6",
	"keypad 7",
	"keypad 8",
	"keypad 9",
	"keypad :",
	"keypad <",
	"keypad = (as400)",
	"keypad =",
	"keypad >",
	"keypad @",
	"keypad ^",
	"keypad a",
	"keypad b",
	"keypad backspace",
	"keypad binary",
	"keypad c",
	"keypad clear",
	"keypad clearentry",
	"keypad d",
	"keypad decimal",
	"keypad e",
	"keypad enter",
	"keypad f",
	"keypad hexadecimal",
	"keypad memadd",
	"keypad memclear",
	"keypad memdivide",
	"keypad memmultiply",
	"keypad memrecall",
	"keypad memstore",
	"keypad memsubtract",
	"keypad octal",
	"keypad space",
	"keypad tab",
	"keypad xor",
	"keypad {",
	"keypad |",
	"keypad ||",
	"keypad }",
	"l",
	"left alt",
	"left ctrl",
	"left gui",
	"left shift",
	"left",
	"m",
	"mail",
	"mediaselect",
	"menu",
	"modeswitch",
	"mute",
	"n",
	"numlock",
	"o",
	"oper",
	"out",
	"p",
	"pagedown",
	"pageup",
	"paste",
	"pause",
	"power",
	"printscreen",
	"prior",
	"q",
	"r",
	"return",
	"right alt",
	"right ctrl",
	"right gui",
	"right shift",
	"right",
	"s",
	"scrolllock",
	"select",
	"separator",
	"sleep",
	"space",
	"stop",
	"sysreq",
	"t",
	"tab",
	"thousandsseparator",
	"u",
	"undo",
	"up",
	"v",
	"volumedown",
	"volumeup",
	"w",
	"www",
	"x",
	"y",
	"z",
};
static const int32_t odLuaBindings_odWindow_key_name_count = (
	sizeof(odLuaBindings_odWindow_key_names) / sizeof(odLuaBindings_odWindow_key_names[0])
);

static bool odLuaBindings_odWindow_get_settings_impl(lua_State* lua, odWindowSettings* settings, int32_t settings_index) {
	if (!OD_CHECK(lua != nullptr)
		|| !OD_CHECK(settings != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return false;
	}

	lua_getfield(lua, settings_index, "width");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		settings->width = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP));
	}

	lua_getfield(lua, settings_index, "height");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		settings->height = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP));
	}

	lua_getfield(lua, settings_index, "fps_limit");
	settings->is_fps_limit_enabled = false;
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		settings->fps_limit = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP));
		settings->is_fps_limit_enabled = true;
	}

	lua_getfield(lua, settings_index, "vsync");
	if ((lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL)) {
		if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TBOOLEAN)) {
			return luaL_error(lua, "settings.vsync must be a boolean or nil");
		}

		settings->is_vsync_enabled = static_cast<bool>(lua_toboolean(lua, OD_LUA_STACK_TOP));
	}

	lua_getfield(lua, settings_index, "visible");
	if ((lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL)) {
		if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TBOOLEAN)) {
			return luaL_error(lua, "settings.visible must be a boolean or nil");
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
static int odLuaBindings_odWindow_new(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int settings_index = 1;
	const int32_t metatable_index = lua_upvalueindex(1);

	luaL_checktype(lua, settings_index, LUA_TTABLE);
	luaL_checktype(lua, metatable_index, LUA_TTABLE);

	lua_getfield(lua, metatable_index, OD_LUA_DEFAULT_NEW_KEY);
	lua_call(lua, /*nargs*/ 0, /*nresults*/ 1);  // call metatable.default_new
	const int self_index = lua_gettop(lua);

	lua_getfield(lua, self_index, "init");
	lua_pushvalue(lua, self_index);
	lua_pushvalue(lua, settings_index);
	lua_call(lua, /*nargs*/ 2, /*nresults*/ 1);

	lua_pushvalue(lua, self_index);
	return 1;
}
static int odLuaBindings_odWindow_set_settings(lua_State* lua) {
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
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

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
	if (settings->is_fps_limit_enabled) {
		lua_pushnumber(lua, static_cast<lua_Number>(settings->fps_limit));
		lua_setfield(lua, settings_index, "fps_limit");
	}
	lua_pushboolean(lua, settings->is_vsync_enabled);
	lua_setfield(lua, settings_index, "vsync");
	lua_pushboolean(lua, settings->is_visible);
	lua_setfield(lua, settings_index, "visible");

	lua_pushvalue(lua, settings_index);
	return 1;
}
static int odLuaBindings_odWindow_get_mouse_state(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_WINDOW));
	if (!OD_CHECK(odWindow_check_valid(window))) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_WINDOW);
	}

	odWindowMouseState state{};
	odWindow_get_mouse_state(window, &state);

	lua_newtable(lua);
	const int state_index = lua_gettop(lua);

	lua_pushnumber(lua, static_cast<lua_Number>(state.x));
	lua_setfield(lua, state_index, "x");
	lua_pushnumber(lua, static_cast<lua_Number>(state.y));
	lua_setfield(lua, state_index, "y");
	lua_pushboolean(lua, state.is_left_down);
	lua_setfield(lua, state_index, "left");
	lua_pushboolean(lua, state.is_middle_down);
	lua_setfield(lua, state_index, "middle");
	lua_pushboolean(lua, state.is_right_down);
	lua_setfield(lua, state_index, "right");

	lua_pushvalue(lua, state_index);
	return 1;
}
static int odLuaBindings_odWindow_get_key_state(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int key_name_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, key_name_index, LUA_TSTRING);

	odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_WINDOW));
	if (!OD_CHECK(odWindow_check_valid(window))) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_WINDOW);
	}

	const char* key_name = luaL_checkstring(lua, key_name_index);

	lua_pushboolean(lua, odWindow_get_key_state(window, key_name));
	return 1;
}
static int odLuaBindings_odWindow_get_key_names(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	lua_newtable(lua);
	const int key_names_index = lua_gettop(lua);

	for (int32_t i = 0; i < odLuaBindings_odWindow_key_name_count; i++) {
		lua_pushstring(lua, odLuaBindings_odWindow_key_names[i]);
		lua_rawseti(lua, key_names_index, i + 1);
	}

	lua_pushvalue(lua, key_names_index);
	return 1;
}
static int odLuaBindings_odWindow_step(lua_State* lua) {
	const int self_index = 1;

	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	// flush any lua output
	fflush(stdout);

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
		|| !OD_CHECK(add_method("new", odLuaBindings_odWindow_new))
		|| !OD_CHECK(add_method("step", odLuaBindings_odWindow_step))
		|| !OD_CHECK(add_method("set_settings", odLuaBindings_odWindow_set_settings))
		|| !OD_CHECK(add_method("get_settings", odLuaBindings_odWindow_get_settings))
		|| !OD_CHECK(add_method("get_mouse_state", odLuaBindings_odWindow_get_mouse_state))
		|| !OD_CHECK(add_method("get_key_state", odLuaBindings_odWindow_get_key_state))
		|| !OD_CHECK(add_method("get_key_names", odLuaBindings_odWindow_get_key_names))) {
		return false;
	}

	return true;
}
