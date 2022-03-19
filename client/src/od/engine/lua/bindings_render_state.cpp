#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/vector.h>
#include <od/core/matrix.h>
#include <od/platform/window.hpp>
#include <od/platform/render_texture.hpp>
#include <od/platform/renderer.hpp>
#include <od/engine/texture_atlas.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

static bool odLuaBindings_odRenderState_get_target_size(lua_State* lua, int settings_index, int32_t* out_width, int32_t* out_height) {
	lua_getfield(lua, settings_index, "target");
	const int target_index = lua_gettop(lua);
	if (!OD_CHECK(lua_type(lua, target_index) == LUA_TUSERDATA)) {
		luaL_error(lua, "settings.target must be of type userdata");  // NOTE: does not return
		return false;
	}

	lua_getfield(lua, target_index, OD_LUA_METATABLE_NAME_KEY);
	const char* target_type = luaL_checkstring(lua, OD_LUA_STACK_TOP);
	if (!OD_CHECK(target_type != nullptr)) {
		luaL_error(lua, "missing settings.target.%s", OD_LUA_METATABLE_NAME_KEY);  // NOTE: does not return
		return false;
	}

	if (strcmp(target_type, OD_LUA_BINDINGS_WINDOW) == 0) {
		odWindow* opt_window = static_cast<odWindow*>(odLua_get_userdata_typed(
			lua, target_index, OD_LUA_BINDINGS_WINDOW));

		if (!OD_CHECK(odWindow_check_valid(opt_window))) {
			luaL_error(lua, "odWindow_check_valid() failed");  // NOTE: does not return
			return false;
		}

		const odWindowSettings* window_settings = odWindow_get_settings(opt_window);
		if (!OD_CHECK(window_settings != nullptr)) {
			luaL_error(lua, "odWindow_get_settings() failed");  // NOTE: does not return
			return false;
		}
		*out_width = window_settings->width;
		*out_height = window_settings->height;
	} else if (strcmp(target_type, OD_LUA_BINDINGS_RENDER_TEXTURE) == 0) {
		odRenderTexture* opt_render_texture = static_cast<odRenderTexture*>(odLua_get_userdata_typed(
			lua, target_index, OD_LUA_BINDINGS_RENDER_TEXTURE));
		if (!OD_CHECK(odRenderTexture_check_valid(opt_render_texture))) {
			luaL_error(lua, "odRenderTexture_check_valid() failed");  // NOTE: does not return
			return false;
		}

		const odTexture* texture = odRenderTexture_get_texture_const(opt_render_texture);
		if (!OD_CHECK(odTexture_check_valid(texture))) {
			luaL_error(lua, "odRenderTexture_get_texture_const() failed");  // NOTE: does not return
			return false;
		}
		if (!OD_CHECK(odTexture_get_size(texture, out_width, out_height))) {
			luaL_error(lua, "odTexture_get_size() failed");  // NOTE: does not return
			return false;
		}
	} else {
		luaL_error(
			lua, "invalid settings.target.%s=%s", OD_LUA_METATABLE_NAME_KEY, target_type);  // NOTE: does not return
		return false;
	}

	return true;
}

static int odLuaBindings_odRenderState_init(lua_State* lua) {
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
	if (!odLuaBindings_odRenderState_get_target_size(lua, settings_index, &width, &height)) {
		return luaL_error(lua, "settings.target parsing failed", OD_LUA_BINDINGS_RENDER_STATE);
	}

	odMatrix view = *odMatrix_get_identity();
	lua_getfield(lua, settings_index, "view");
	const int view_index = lua_gettop(lua);
	if (lua_type(lua, view_index) != LUA_TNIL) {
		if (!OD_CHECK(lua_type(lua, view_index) == LUA_TTABLE)) {
			return luaL_error(lua, "settings.view must be of type table");
		}
		if (!OD_CHECK(odLua_get_matrix(lua, view_index, &view))) {
			return luaL_error(lua, "settings.view validation failed");
		}
	}

	odMatrix projection = *odMatrix_get_identity();
	lua_getfield(lua, settings_index, "projection");
	const int projection_index = lua_gettop(lua);
	if (lua_type(lua, projection_index) != LUA_TNIL) {
		if (!OD_CHECK(lua_type(lua, projection_index) == LUA_TTABLE)) {
			return luaL_error(lua, "settings.projection must be of type table");
		}
		if (!OD_CHECK(odLua_get_matrix(lua, projection_index, &projection))) {
			return luaL_error(lua, "settings.projection validation failed");
		}
	}

	odBounds viewport{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)};
	lua_getfield(lua, settings_index, "viewport");
	const int viewport_index = lua_gettop(lua);
	if (lua_type(lua, projection_index) != LUA_TNIL) {
		if (!OD_CHECK(odLua_get_bounds(lua, viewport_index, &render_state->viewport))) {
			return luaL_error(lua, "viewport validation failed");
		}
	}

	*render_state = odRenderState{view, projection, viewport};

	if (!OD_CHECK(odRenderState_check_valid(render_state))) {
		return luaL_error(lua, "render state validation failed");
	}

	return 0;
}
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

	lua_getfield(lua, self_index, "init");
	lua_pushvalue(lua, self_index);
	lua_pushvalue(lua, settings_index);
	lua_call(lua, /*nargs*/ 2, /*nresults*/ 1);

	int32_t width = static_cast<int32_t>(render_state->viewport.x2);
	int32_t height = static_cast<int32_t>(render_state->viewport.y2);

	render_state->projection = odMatrix{};
	odMatrix_init_ortho_2d(&render_state->projection, width, height);

	return 0;
}
static int odLuaBindings_odRenderState_assign(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int other_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, other_index, LUA_TUSERDATA);

	odRenderState* render_state = static_cast<odRenderState*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_RENDER_STATE));
	if (!OD_CHECK(render_state != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDER_STATE);
	}

	const odRenderState* other_render_state = static_cast<odRenderState*>(odLua_get_userdata_typed(lua, other_index, OD_LUA_BINDINGS_RENDER_STATE));
	if (!OD_CHECK(render_state != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDER_STATE);
	}

	if (!OD_CHECK(odRenderState_check_valid(other_render_state))) {
		return luaL_error(lua, "render state validation failed");
	}

	*render_state = *other_render_state;

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

	*render_state = odRenderState{};

	return 0;
}
static int odLuaBindings_odRenderState_new(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int settings_index = 1;

	luaL_checktype(lua, settings_index, LUA_TTABLE);

	const int32_t metatable_index = lua_upvalueindex(1);

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
static int odLuaBindings_odRenderState_new_ortho_2d(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int settings_index = 1;

	luaL_checktype(lua, settings_index, LUA_TTABLE);

	const int32_t metatable_index = lua_upvalueindex(1);

	luaL_checktype(lua, metatable_index, LUA_TTABLE);

	lua_getfield(lua, metatable_index, OD_LUA_DEFAULT_NEW_KEY);
	lua_call(lua, /*nargs*/ 0, /*nresults*/ 1);  // call metatable.default_new
	const int self_index = lua_gettop(lua);

	lua_getfield(lua, self_index, "init_ortho_2d");
	lua_pushvalue(lua, self_index);
	lua_pushvalue(lua, settings_index);
	lua_call(lua, /*nargs*/ 2, /*nresults*/ 1);

	lua_pushvalue(lua, self_index);
	return 1;
}
static int odLuaBindings_odRenderState_copy(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int other_index = 1;

	luaL_checktype(lua, other_index, LUA_TUSERDATA);

	const int32_t metatable_index = lua_upvalueindex(1);

	luaL_checktype(lua, metatable_index, LUA_TTABLE);

	lua_getfield(lua, metatable_index, OD_LUA_DEFAULT_NEW_KEY);
	lua_call(lua, /*nargs*/ 0, /*nresults*/ 1);  // call metatable.default_new
	const int self_index = lua_gettop(lua);

	lua_getfield(lua, self_index, "assign");
	lua_pushvalue(lua, self_index);
	lua_pushvalue(lua, other_index);
	lua_call(lua, /*nargs*/ 2, /*nresults*/ 1);

	lua_pushvalue(lua, self_index);
	return 1;
}
static int odLuaBindings_odRenderState_transform_view(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int view_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, view_index, LUA_TTABLE);

	odRenderState* render_state = static_cast<odRenderState*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_RENDER_STATE));
	if (!OD_CHECK(render_state != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDER_STATE);
	}

	odMatrix transform = *odMatrix_get_identity();
	if (!OD_CHECK(odLua_get_matrix(lua, view_index, &transform))) {
			return luaL_error(lua, "view matrix validation failed");
	}

	odMatrix_multiply(&render_state->view, &transform);

	if (!OD_CHECK(odRenderState_check_valid(render_state))) {
		return luaL_error(lua, "render state validation failed");
	}

	return 0;
}
static int odLuaBindings_odRenderState_transform_projection(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int projection_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, projection_index, LUA_TTABLE);

	odRenderState* render_state = static_cast<odRenderState*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_RENDER_STATE));
	if (!OD_CHECK(render_state != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDER_STATE);
	}

	odMatrix transform = *odMatrix_get_identity();
	if (!OD_CHECK(odLua_get_matrix(lua, projection_index, &transform))) {
		return luaL_error(lua, "projection matrix validation failed");
	}

	odMatrix_multiply(&render_state->projection, &transform);

	if (!OD_CHECK(odRenderState_check_valid(render_state))) {
		return luaL_error(lua, "render state validation failed");
	}

	return 0;
}
static int odLuaBindings_odRenderState_set_viewport(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odRenderState* render_state = static_cast<odRenderState*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_RENDER_STATE));
	if (!OD_CHECK(render_state != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDER_STATE);
	}

	lua_getfield(lua, settings_index, "viewport");
	const int viewport_index = lua_gettop(lua);
	if (!OD_CHECK(odLua_get_bounds(lua, viewport_index, &render_state->viewport))) {
		return luaL_error(lua, "viewport validation failed");
	}

	if (!OD_CHECK(odRenderState_check_valid(render_state))) {
		return luaL_error(lua, "render state validation failed");
	}

	return 0;
}
static int odLuaBindings_odRenderState_set_viewport_ortho_2d(lua_State* lua) {
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
	if (!odLuaBindings_odRenderState_get_target_size(lua, settings_index, &width, &height)) {
		return luaL_error(lua, "settings.target parsing failed", OD_LUA_BINDINGS_RENDER_STATE);
	}
	float height_f = static_cast<float>(height);

	lua_getfield(lua, settings_index, "viewport");
	odBounds viewport_inverted_y{};
	const int viewport_index = lua_gettop(lua);
	if (!OD_CHECK(odLua_get_bounds(lua, viewport_index, &viewport_inverted_y))) {
		return luaL_error(lua, "viewport validation failed");
	}

	render_state->viewport = viewport_inverted_y;
	render_state->viewport.y1 = height_f - viewport_inverted_y.y2;
	render_state->viewport.y2 = height_f - viewport_inverted_y.y1;

	if (!OD_CHECK(odRenderState_check_valid(render_state))) {
		return luaL_error(lua, "render state validation failed");
	}

	return 0;
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
	if (!OD_CHECK(add_method("init", odLuaBindings_odRenderState_init))
		|| !OD_CHECK(add_method("init_ortho_2d", odLuaBindings_odRenderState_init_ortho_2d))
		|| !OD_CHECK(add_method("assign", odLuaBindings_odRenderState_assign))
		|| !OD_CHECK(add_method("destroy", odLuaBindings_odRenderState_destroy))
		|| !OD_CHECK(add_method("new", odLuaBindings_odRenderState_new))
		|| !OD_CHECK(add_method("new_ortho_2d", odLuaBindings_odRenderState_new_ortho_2d))
		|| !OD_CHECK(add_method("copy", odLuaBindings_odRenderState_copy))
		|| !OD_CHECK(add_method("transform_view", odLuaBindings_odRenderState_transform_view))
		|| !OD_CHECK(add_method("transform_projection", odLuaBindings_odRenderState_transform_projection))
		|| !OD_CHECK(add_method("set_viewport", odLuaBindings_odRenderState_set_viewport))
		|| !OD_CHECK(add_method("set_viewport_ortho_2d", odLuaBindings_odRenderState_set_viewport_ortho_2d))) {
		return false;
	}

	return true;
}
