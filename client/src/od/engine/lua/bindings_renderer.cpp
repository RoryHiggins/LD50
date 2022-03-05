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
static int odLuaBindings_odRenderer_draw_vertices(lua_State* lua) {
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

	lua_getfield(lua, settings_index, "vertices");
	const int vertices_index = lua_gettop(lua);
	if (!OD_CHECK(lua_type(lua, vertices_index) == LUA_TTABLE)) {
		return luaL_error(lua, "settings.vertices must be of type table");
	}

	int vertices_count = odLua_get_length(lua, vertices_index);
	OD_DEBUG("vertices_count=%d", vertices_count);
	if (!OD_DEBUG_CHECK(vertices_count % 3 == 0)) {
			return luaL_error(lua, "settings.vertices length must be divisible by 3 (only supports triangles)");
		}

	static odArrayT<odVertex> vertices;
	if (!OD_CHECK(vertices.set_count(vertices_count))) {
		return luaL_error(lua, "vertices.set_count(%d) failed", vertices_count);
	}
	odVertex* vertices_raw = vertices.begin();

	for (int i = 1; i <= vertices_count; i++) {
		lua_rawgeti(lua, vertices_index, i);
		const int vertex_index = lua_gettop(lua);

		if (!OD_DEBUG_CHECK(lua_type(lua, vertex_index) == LUA_TTABLE)) {
			return luaL_error(lua, "settings.vertices[%d] must be of type table", i);
		}

		const int values_count = 10;
		if (!OD_DEBUG_CHECK(odLua_get_length(lua, vertex_index) == 10)) {
			return luaL_error(lua, "settings.vertices[%d] must provide 10 elements", i);
		}

		lua_Number values[values_count + 1];  // over allocate to align with lua's 1-indexing

		for (int j = 1; j <= values_count; j++) {
			lua_rawgeti(lua, vertex_index, j);
			if (!OD_DEBUG_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TNUMBER)) {
				return luaL_error(lua, "settings.vertices[%d][%d] must be of type number", i, j);
			}

			values[j] = lua_tonumber(lua, OD_LUA_STACK_TOP);
		}

		lua_settop(lua, vertex_index);

		odVertex vertex = {
			odVector{
				static_cast<float>(values[1]),
				static_cast<float>(values[2]),
				static_cast<float>(values[3]),
				static_cast<float>(values[4]),
			},
			odColor{
				static_cast<uint8_t>(values[5]),
				static_cast<uint8_t>(values[6]),
				static_cast<uint8_t>(values[7]),
				static_cast<uint8_t>(values[8]),
			},
			static_cast<float>(values[9]),
			static_cast<float>(values[10])
		};

		for (int j = 5; j < 9; j++) {
			if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(values[j])))) {
				return luaL_error(
					lua, "settings.vertices[%d][%d] (color) must be an integer value in the range [0, 255]", i, j - 4);
			}
		}

		if (!OD_DEBUG_CHECK(odVertex_check_valid_3d(&vertex))) {
			return luaL_error(
				lua, "settings.vertices[%d] not valid", 1, 4);
		}

		vertices_raw[i - 1] = vertex;
	}

	if (!OD_CHECK(odRenderer_draw_vertices(renderer, render_state, vertices_raw, vertices_count))) {
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
		|| !OD_CHECK(add_method("flush", odLuaBindings_odRenderer_flush))
		|| !OD_CHECK(add_method("clear", odLuaBindings_odRenderer_clear))
		|| !OD_CHECK(add_method("draw_vertices", odLuaBindings_odRenderer_draw_vertices))) {
		return false;
	}

	return true;
}
