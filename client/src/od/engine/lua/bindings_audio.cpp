#include <od/engine/lua/bindings.h>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/math.h>
#include <od/platform/file.h>
#include <od/platform/audio.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

static int odLuaBindings_odAudio_init(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odAudio* audio = static_cast<odAudio*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_AUDIO));

	if (!OD_CHECK(audio != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_AUDIO);
	}

	if (!OD_CHECK(odAudio_init(audio))) {
		return luaL_error(lua, "odAudio_init() failed");
	}

	return 0;
}
static int odLuaBindings_odAudio_init_wav_file(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odAudio* audio = static_cast<odAudio*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_AUDIO));

	if (!OD_CHECK(audio != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_AUDIO);
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

	if (!OD_CHECK(odAudio_init_wav_file(audio, filename))) {
		return luaL_error(lua, "odAudio_init() failed, filename=%s", filename);
	}

	return 0;
}
static int odLuaBindings_odAudio_new(lua_State* lua) {
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
static int odLuaBindings_odAudio_new_wav_file(lua_State* lua) {
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

	lua_getfield(lua, self_index, "init_wav_file");
	lua_pushvalue(lua, self_index);
	lua_pushvalue(lua, settings_index);
	lua_call(lua, /*nargs*/ 2, /*nresults*/ 1);

	lua_pushvalue(lua, self_index);
	return 1;
}
static int odLuaBindings_odAudio_destroy(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	odAudio* audio = static_cast<odAudio*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_AUDIO));
	if (!OD_CHECK(audio != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_AUDIO);
	}

	odAudio_destroy(audio);

	return 0;
}
static int odLuaBindings_odAudio_set_volume(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odAudio* audio = static_cast<odAudio*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_AUDIO));

	if (!OD_CHECK(audio != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_AUDIO);
	}

	lua_getfield(lua, settings_index, "volume");
	float volume = static_cast<float>(luaL_checknumber(lua, OD_LUA_STACK_TOP));
	if (!odFloat_is_normalized(volume)) {
		return luaL_error(lua, "settings.volume must be a number between [0.0, 1.0] inclusive");
	}

	return 0;
}
static int odLuaBindings_odAudio_play(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odAudio* audio = static_cast<odAudio*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_AUDIO));

	if (!OD_CHECK(audio != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_AUDIO);
	}

	odAudioPlaybackSettings settings = *odAudioPlaybackSettings_get_defaults();

	lua_getfield(lua, settings_index, "loop_count");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		settings.loop_count = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP));
		if (!OD_CHECK(settings.loop_count >= 0)
			|| !OD_DEBUG_CHECK(odFloat_is_precise_int24(static_cast<float>(luaL_checknumber(lua, OD_LUA_STACK_TOP))))) {
			return luaL_error(lua, "settings.loop_count must be positive integer");
		}
	}

	lua_getfield(lua, settings_index, "cutoff_time");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		settings.cutoff_time_ms = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP) * 1000.0);
		settings.is_cutoff_time_enabled = true;
		if (!OD_CHECK(settings.cutoff_time_ms >= 0)) {
			return luaL_error(lua, "settings.cutoff_time_ms must be positive integer");
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

	int32_t channel = odAudio_play(audio, &settings);
	if (!OD_CHECK(channel >= 0)) {
		// lack of free channels is bad but not something that should raise a lua error
		if (channel != OD_AUDIO_PLAYBACK_ID_NO_CHANNELS) {
			return luaL_error(lua, "odAudio_play() failed");
		}
	}

	lua_pushnumber(lua, channel);
	return 1;
}
static int odLuaBindings_odAudio_stop(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int playback_id_index = 1;

	odAudioPlaybackId playback_id = static_cast<odAudioPlaybackId>(luaL_checknumber(lua, playback_id_index));
	if (!OD_CHECK(odAudioPlaybackId_check_valid(playback_id))) {
		lua_pushboolean(lua, false);
		return 1;
	}

	lua_pushboolean(lua, odAudio_stop(playback_id));
	return 1;
}
static int odLuaBindings_odAudio_stop_all(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	luaL_checktype(lua, 1, LUA_TNONE);

	OD_DISCARD(OD_CHECK(odAudio_stop_all()));
	return 0;
}
static int odLuaBindings_odAudio_is_playing(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int playback_id_index = 1;

	odAudioPlaybackId playback_id = static_cast<odAudioPlaybackId>(luaL_checknumber(lua, playback_id_index));
	if (!OD_CHECK(odAudioPlaybackId_check_valid(playback_id))) {
		lua_pushboolean(lua, false);
		return 1;
	}

	lua_pushboolean(lua, odAudio_is_playing(playback_id));
	return 1;
}
bool odLuaBindings_odAudio_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_declare(lua, OD_LUA_BINDINGS_AUDIO))
		|| !OD_CHECK(odLua_metatable_set_new_delete(lua, OD_LUA_BINDINGS_AUDIO, odType_get<odAudio>()))) {
		return false;
	}

	auto add_method = [lua](const char* name, odLuaFn* fn) -> bool {
		return odLua_metatable_set_function(lua, OD_LUA_BINDINGS_AUDIO, name, fn);
	};
	if (!OD_CHECK(add_method("init", odLuaBindings_odAudio_init))
		|| !OD_CHECK(add_method("init_wav_file", odLuaBindings_odAudio_init_wav_file))
		|| !OD_CHECK(add_method("new", odLuaBindings_odAudio_new))
		|| !OD_CHECK(add_method("new_wav_file", odLuaBindings_odAudio_new_wav_file))
		|| !OD_CHECK(add_method("destroy", odLuaBindings_odAudio_destroy))
		|| !OD_CHECK(add_method("set_volume", odLuaBindings_odAudio_set_volume))
		|| !OD_CHECK(add_method("play", odLuaBindings_odAudio_play))
		|| !OD_CHECK(add_method("stop", odLuaBindings_odAudio_stop))
		|| !OD_CHECK(add_method("stop_all", odLuaBindings_odAudio_stop_all))
		|| !OD_CHECK(add_method("is_playing", odLuaBindings_odAudio_is_playing))) {
		return false;
	}

	return true;
}
