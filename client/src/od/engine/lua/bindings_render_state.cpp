#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/platform/window.hpp>
#include <od/platform/render_texture.hpp>
#include <od/platform/renderer.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

static int odLuaBindings_odRenderState_init_ortho_2d(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odRenderState* render_state = static_cast<odRenderState*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_RENDER_STATE));
	if (!OD_CHECK(render_state != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDER_STATE);
	}

	int32_t width = 0;
	int32_t height = 0;
	odRenderTexture* opt_render_texture = nullptr;

	lua_getfield(lua, settings_index, "src");
	const int src_index = lua_gettop(lua);
	if (!OD_CHECK(lua_type(lua, src_index) == LUA_TUSERDATA)) {
		return luaL_error(lua, "settings.src must be of type userdata");
	}

	lua_getfield(lua, settings_index, "target");
	const int target_index = lua_gettop(lua);
	if (!OD_CHECK(lua_type(lua, target_index) == LUA_TUSERDATA)) {
		return luaL_error(lua, "settings.target must be of type userdata");
	}

	lua_getfield(lua, target_index, OD_LUA_METATABLE_NAME_KEY);
	const char* target_type = luaL_checkstring(lua, OD_LUA_STACK_TOP);
	if (!OD_CHECK(target_type != nullptr)) {
		return luaL_error(lua, "missing settings.target.%s", OD_LUA_METATABLE_NAME_KEY);
	}

	if (strcmp(target_type, OD_LUA_BINDINGS_WINDOW) == 0) {
		const odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(
			lua, target_index, OD_LUA_BINDINGS_WINDOW));

		if (!OD_CHECK(odWindow_check_valid(window))) {
			return luaL_error(lua, "odWindow_check_valid() failed");
		}

		const odWindowSettings* window_settings = odWindow_get_settings(window);
		if (!OD_CHECK(window_settings != nullptr)) {
			return luaL_error(lua, "odWindow_get_settings() failed");
		}
		width = window_settings->width;
		height = window_settings->height;
	} else if (strcmp(target_type, OD_LUA_BINDINGS_RENDER_TEXTURE) == 0) {
		opt_render_texture = static_cast<odRenderTexture*>(odLua_get_userdata_typed(
			lua, target_index, OD_LUA_BINDINGS_RENDER_TEXTURE));
		if (!OD_CHECK(odRenderTexture_check_valid(opt_render_texture))) {
			return luaL_error(lua, "odRenderTexture_check_valid() failed");
		}

		const odTexture* texture = odRenderTexture_get_texture_const(opt_render_texture);
		if (!OD_CHECK(odTexture_check_valid(texture))) {
			return luaL_error(lua, "odRenderTexture_get_texture_const() failed");
		}
		if (!OD_CHECK(odTexture_get_size(texture, &width, &height))) {
			return luaL_error(lua, "odTexture_get_size() failed");
		}
	} else {
		return luaL_error(
			lua, "invalid settings.target.%s=%s", OD_LUA_METATABLE_NAME_KEY, target_type);
	}

	lua_getfield(lua, src_index, OD_LUA_METATABLE_NAME_KEY);
	const char* src_type = luaL_checkstring(lua, OD_LUA_STACK_TOP);
	if (!OD_CHECK(src_type != nullptr)) {
		return luaL_error(lua, "missing settings.src.%s", OD_LUA_METATABLE_NAME_KEY);
	}

	const odTexture* src_texture = nullptr;
	if (strcmp(src_type, OD_LUA_BINDINGS_TEXTURE) == 0) {
		src_texture = static_cast<const odTexture*>(odLua_get_userdata_typed(
			lua, src_index, OD_LUA_BINDINGS_TEXTURE));
	} else if (strcmp(src_type, OD_LUA_BINDINGS_RENDER_TEXTURE) == 0) {
		const odRenderTexture* src_render_texture = static_cast<odRenderTexture*>(odLua_get_userdata_typed(
			lua, src_index, OD_LUA_BINDINGS_RENDER_TEXTURE));
		if (!OD_CHECK(odRenderTexture_check_valid(src_render_texture))) {
			return luaL_error(lua, "odRenderTexture_check_valid() failed");
		}

		src_texture = odRenderTexture_get_texture_const(opt_render_texture);
	} else {
		return luaL_error(lua, "invalid settings.src.%s=%s", OD_LUA_METATABLE_NAME_KEY, src_type);
	}
	if (!OD_CHECK(odTexture_check_valid(src_texture))) {
		return luaL_error(lua, "odRenderTexture_get_texture_const() failed");
	}

	odMatrix projection{};
	odMatrix_init_ortho_2d(&projection, width, height);

	*render_state = odRenderState{
		*odMatrix_get_identity(),
		projection,
		odBounds{
			0.0f,
			0.0f,
			static_cast<float>(width),
			static_cast<float>(height)
		},
		src_texture,
		opt_render_texture,
	};

	return 0;
}
static int odLuaBindings_odRenderState_destroy(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	odRenderState* render_state = static_cast<odRenderState*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_TEXTURE));

	if (!OD_CHECK(render_state != nullptr)) {
		return 0;
	}

	*render_state = odRenderState{
		odMatrix{},
		odMatrix{},
		odBounds{},
		nullptr,
		nullptr
	};

	return 0;
}
static int odLuaBindings_odRenderState_new_ortho_2d(lua_State* lua) {
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

	lua_getfield(lua, self_index, "init_ortho_2d");
	if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TFUNCTION)) {
		return luaL_error(lua, "metatable.init must be of type function");
	}

	lua_pushvalue(lua, self_index);
	lua_pushvalue(lua, settings_index);
	lua_call(lua, /*nargs*/ 2, /*nresults*/ 1);

	lua_pushvalue(lua, self_index);
	return 1;
}
bool odLuaBindings_odRenderState_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_declare(lua, OD_LUA_BINDINGS_RENDER_STATE))
		|| !OD_CHECK(odLua_metatable_set_new_delete(lua, OD_LUA_BINDINGS_RENDER_STATE, odType_get<odRenderState>()))) {
		return false;
	}

	auto add_method = [lua](const char* name, odLuaFn* fn) -> bool {
		return odLua_metatable_set_function(lua, OD_LUA_BINDINGS_RENDER_STATE, name, fn);
	};
	if (!OD_CHECK(add_method("init_ortho_2d", odLuaBindings_odRenderState_init_ortho_2d))
		|| !OD_CHECK(add_method("destroy", odLuaBindings_odRenderState_destroy))
		|| !OD_CHECK(add_method("new_ortho_2d", odLuaBindings_odRenderState_new_ortho_2d))) {
		return false;
	}

	return true;
}
