#include <od/engine/lua/bindings.h>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/math.h>
#include <od/platform/file.h>
#include <od/platform/music.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

static int odLuaBindings_odMusic_init(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odMusic* music = static_cast<odMusic*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_MUSIC));

	if (!OD_CHECK(music != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_MUSIC);
	}

	if (!OD_CHECK(odMusic_init(music))) {
		return luaL_error(lua, "odMusic_init() failed");
	}

	return 0;
}
static int odLuaBindings_odMusic_init_file(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odMusic* music = static_cast<odMusic*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_MUSIC));

	if (!OD_CHECK(music != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_MUSIC);
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

	if (!OD_CHECK(odMusic_init_file(music, filename))) {
		return luaL_error(lua, "odMusic_init_file(%s) failed", filename);
	}

	return 0;
}
static int odLuaBindings_odMusic_new(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int settings_index = 1;
	const int metatable_index = lua_upvalueindex(1);

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
static int odLuaBindings_odMusic_new_file(lua_State* lua) {
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

	lua_getfield(lua, self_index, "init_file");
	lua_pushvalue(lua, self_index);
	lua_pushvalue(lua, settings_index);
	lua_call(lua, /*nargs*/ 2, /*nresults*/ 1);

	lua_pushvalue(lua, self_index);
	return 1;
}
static int odLuaBindings_odMusic_destroy(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	odMusic* music = static_cast<odMusic*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_MUSIC));
	if (!OD_CHECK(music != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_MUSIC);
	}

	odMusic_destroy(music);

	return 0;
}
static int odLuaBindings_odMusic_play(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odMusic* music = static_cast<odMusic*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_MUSIC));

	if (!OD_CHECK(music != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_MUSIC);
	}

	odMusicPlaybackSettings settings = *odMusicPlaybackSettings_get_defaults();

	lua_getfield(lua, settings_index, "loop_count");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		settings.loop_count = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP));
		if (!OD_CHECK(settings.loop_count >= 0)
			|| !OD_DEBUG_CHECK(odFloat_is_precise_int24(static_cast<float>(luaL_checknumber(lua, OD_LUA_STACK_TOP))))) {
			return luaL_error(lua, "settings.loop_count must be positive integer");
		}
	}

	lua_getfield(lua, settings_index, "fadein_time");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		settings.fadein_time_ms = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP) * 1000.0);
		if (!OD_CHECK(settings.fadein_time_ms >= 0)) {
			return luaL_error(lua, "settings.fadein_time_ms must be positive integer");
		}
	}

	lua_getfield(lua, settings_index, "volume");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		settings.volume = static_cast<float>(luaL_checknumber(lua, OD_LUA_STACK_TOP));
		if (!OD_CHECK(odFloat_is_normalized(settings.volume))) {
			return luaL_error(lua, "settings.volume must be a number between [0.0, 1.0] inclusive");
		}
	}

	lua_getfield(lua, settings_index, "loop_forever");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		settings.is_loop_forever_enabled = lua_toboolean(lua, OD_LUA_STACK_TOP);
		if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TBOOLEAN)) {
			return luaL_error(lua, "settings.loop_forever must be a boolean or nil");
		}

		lua_getfield(lua, settings_index, "loop_count");
		if (!OD_CHECK((!settings.is_loop_forever_enabled) || (lua_type(lua, OD_LUA_STACK_TOP) == LUA_TNIL))) {
			OD_WARN("settings.loop_count ignored as settings.loop_forever is true");
		}
	}

	// TODO remaining settings

	if (!OD_CHECK(odMusic_play(music, &settings))) {
		return luaL_error(lua, "odMusic_play() failed");
	}

	return 0;
}
static int odLuaBindings_odMusic_stop(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	odMusic_stop();
	return 0;
}
static int odLuaBindings_odMusic_pause(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	odMusic_pause();
	return 0;
}
static int odLuaBindings_odMusic_resume(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	odMusic_resume();
	return 0;
}
static int odLuaBindings_odMusic_is_playing(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	lua_pushboolean(lua, odMusic_is_playing());
	return 1;
}
static int odLuaBindings_odMusic_is_paused(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	lua_pushboolean(lua, odMusic_is_paused());
	return 1;
}
bool odLuaBindings_odMusic_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_declare(lua, OD_LUA_BINDINGS_MUSIC))
		|| !OD_CHECK(odLua_metatable_set_new_delete(lua, OD_LUA_BINDINGS_MUSIC, odType_get<odMusic>()))) {
		return false;
	}

	auto add_method = [lua](const char* name, odLuaFn* fn) -> bool {
		return odLua_metatable_set_function(lua, OD_LUA_BINDINGS_MUSIC, name, fn);
	};
	if (!OD_CHECK(add_method("init", odLuaBindings_odMusic_init))
		|| !OD_CHECK(add_method("init_file", odLuaBindings_odMusic_init_file))
		|| !OD_CHECK(add_method("new", odLuaBindings_odMusic_new))
		|| !OD_CHECK(add_method("new_file", odLuaBindings_odMusic_new_file))
		|| !OD_CHECK(add_method("destroy", odLuaBindings_odMusic_destroy))
		|| !OD_CHECK(add_method("play", odLuaBindings_odMusic_play))
		|| !OD_CHECK(add_method("stop", odLuaBindings_odMusic_stop))
		|| !OD_CHECK(add_method("pause", odLuaBindings_odMusic_pause))
		|| !OD_CHECK(add_method("resume", odLuaBindings_odMusic_resume))
		|| !OD_CHECK(add_method("is_playing", odLuaBindings_odMusic_is_playing))
		|| !OD_CHECK(add_method("is_paused", odLuaBindings_odMusic_is_paused))) {
		return false;
	}

	return true;
}
