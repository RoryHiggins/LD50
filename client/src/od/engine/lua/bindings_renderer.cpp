#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/math.h>
#include <od/core/type.hpp>
#include <od/core/color.h>
#include <od/core/vector.h>
#include <od/core/vertex.h>
#include <od/core/array.hpp>
#include <od/platform/renderer.hpp>
#include <od/platform/window.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.hpp>

static int odLuaBindings_odRenderer_init(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odRenderer* renderer = static_cast<odRenderer*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_RENDERER));
	if (!OD_CHECK(renderer != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDERER);
	}

	lua_getfield(lua, settings_index, "window");
	const int window_index = lua_gettop(lua);

	odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(lua, window_index, OD_LUA_BINDINGS_WINDOW));
	if (!OD_CHECK(odWindow_check_valid(window))) {
		return luaL_error(lua, "odWindow_check_valid() failed");
	}

	if (!OD_CHECK(odRenderer_init(renderer, window))) {
		return luaL_error(lua, "odRenderer_init() failed");
	}

	return 0;
}
static int odLuaBindings_odRenderer_destroy(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	odRenderer* renderer = static_cast<odRenderer*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_RENDERER));
	if (!OD_CHECK(renderer != nullptr)) {
		return 0;
	}

	odRenderer_destroy(renderer);

	return 0;
}
static int odLuaBindings_odRenderer_new(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int settings_index = 1;
	const int32_t metatable_index = lua_upvalueindex(1);

	luaL_checktype(lua, settings_index, LUA_TTABLE);
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
static int odLuaBindings_odRenderer_flush(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odRenderer* renderer = static_cast<odRenderer*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_RENDERER));
	if (!OD_CHECK(renderer != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDERER);
	}

	if (!OD_CHECK(odRenderer_flush(renderer))) {
		return luaL_error(lua, "odRenderer_flush() failed");
	}

	return 0;
}
static int odLuaBindings_odRenderer_clear(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odRenderer* renderer = static_cast<odRenderer*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_RENDERER));
	if (!OD_CHECK(renderer != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDERER);
	}

	lua_getfield(lua, settings_index, "render_state");
	const int window_index = lua_gettop(lua);
	odRenderState* render_state = static_cast<odRenderState*>(odLua_get_userdata_typed(
		lua, window_index, OD_LUA_BINDINGS_RENDER_STATE));
	if (!OD_CHECK(render_state != nullptr)) {
		return luaL_error(lua, "settings.render_state invalid");
	}

	lua_getfield(lua, settings_index, "color");
	const int color_index = lua_gettop(lua);
	if (!OD_CHECK(lua_type(lua, color_index) == LUA_TTABLE)) {
		return luaL_error(lua, "settings.color must be of type table");
	}

	int color_coord_count = odLua_get_length(lua, color_index);
	if (!OD_CHECK(color_coord_count == 4)) {
		return luaL_error(lua, "settings.color must have 4 elements (rgba)");
	}

	odColor color{};
	uint8_t* color_bytes = reinterpret_cast<uint8_t*>(&color);
	for (int i = 1; i <= 4; i++) {
		lua_rawgeti(lua, color_index, i);
		lua_Number coord_value = lua_tonumber(lua, OD_LUA_STACK_TOP);
		if (!OD_CHECK(odFloat_is_precise_uint8(static_cast<float>(coord_value)))) {
			return luaL_error(
				lua, "settings.color[%d] must be an integer value in the range [0, 255]", i);
		}

		color_bytes[i - 1] = static_cast<uint8_t>(lua_tonumber(lua, OD_LUA_STACK_TOP));
	}

	if (!OD_CHECK(odRenderer_clear(renderer, render_state, &color))) {
		return luaL_error(lua, "odRenderer_clear() failed");
	}

	return 0;
}
static int odLuaBindings_odRenderer_draw_vertex_array(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odRenderer* renderer = static_cast<odRenderer*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_RENDERER));
	if (!OD_CHECK(renderer != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_RENDERER);
	}

	lua_getfield(lua, settings_index, "render_state");
	const int window_index = lua_gettop(lua);
	odRenderState* render_state = static_cast<odRenderState*>(odLua_get_userdata_typed(
		lua, window_index, OD_LUA_BINDINGS_RENDER_STATE));
	if (!OD_CHECK(render_state != nullptr)) {
		return luaL_error(lua, "settings.render_state invalid");
	}

	lua_getfield(lua, settings_index, "vertex_array");
	const int vertex_array_index = lua_gettop(lua);
	if (!OD_CHECK(lua_type(lua, vertex_array_index) == LUA_TUSERDATA)) {
		return luaL_error(lua, "settings.vertex_array must be of type userdata");
	}

	odTrivialArrayT<odVertex>* vertex_array = static_cast<odTrivialArrayT<odVertex>*>(odLua_get_userdata_typed(
		lua, vertex_array_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return 0;
	}

	if (!OD_CHECK(odRenderer_draw_vertices(renderer, render_state, vertex_array->begin(), vertex_array->get_count()))) {
		return luaL_error(lua, "odRenderer_draw_vertices() failed");
	}

	return 0;
}
bool odLuaBindings_odRenderer_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_declare(lua, OD_LUA_BINDINGS_RENDERER))
		|| !OD_CHECK(odLua_metatable_set_new_delete(lua, OD_LUA_BINDINGS_RENDERER, odType_get<odRenderer>()))) {
		return false;
	}

	auto add_method = [lua](const char* name, odLuaFn* fn) -> bool {
		return odLua_metatable_set_function(lua, OD_LUA_BINDINGS_RENDERER, name, fn);
	};
	if (!OD_CHECK(add_method("init", odLuaBindings_odRenderer_init))
		|| !OD_CHECK(add_method("destroy", odLuaBindings_odRenderer_destroy))
		|| !OD_CHECK(add_method("new", odLuaBindings_odRenderer_new))
		|| !OD_CHECK(add_method("flush", odLuaBindings_odRenderer_flush))
		|| !OD_CHECK(add_method("clear", odLuaBindings_odRenderer_clear))
		|| !OD_CHECK(add_method("draw_vertex_array", odLuaBindings_odRenderer_draw_vertex_array))) {
		return false;
	}

	return true;
}
