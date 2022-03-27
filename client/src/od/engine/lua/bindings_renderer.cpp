#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/math.h>
#include <od/core/type.hpp>
#include <od/core/color.h>
#include <od/core/vector.h>
#include <od/core/vertex.h>
#include <od/core/array.hpp>
#include <od/platform/texture.hpp>
#include <od/platform/render_texture.hpp>
#include <od/platform/renderer.hpp>
#include <od/platform/window.hpp>
#include <od/engine/texture_atlas.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.hpp>

static odRenderTexture* odLuaBindings_odRenderer_get_render_texture_impl(lua_State* lua, int settings_index, const odRenderer* renderer) {
	if (!OD_CHECK(lua != nullptr)) {
		return nullptr;
	}

	luaL_checktype(lua, settings_index, LUA_TTABLE);

	lua_getfield(lua, settings_index, "target");
	const int target_index = lua_gettop(lua);

	if (lua_type(lua, target_index) == LUA_TNIL) {
		return nullptr;
	}

	lua_getfield(lua, target_index, OD_LUA_METATABLE_NAME_KEY);
	const char* target_type = luaL_checkstring(lua, OD_LUA_STACK_TOP);

	odRenderTexture* opt_render_texture = nullptr;
	if (strcmp(target_type, OD_LUA_BINDINGS_WINDOW) == 0) {
		if (OD_BUILD_DEBUG) {
			const odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(
				lua, target_index, OD_LUA_BINDINGS_WINDOW));
			if (!OD_DEBUG_CHECK(window == renderer->window)) {
				luaL_error(lua, "settings.target is a window that doesn't match renderer.window");    // NOTE: does not return
			}
			OD_MAYBE_UNUSED(window);
		}
	} else if (strcmp(target_type, OD_LUA_BINDINGS_RENDER_TEXTURE) == 0) {
		opt_render_texture = static_cast<odRenderTexture*>(odLua_get_userdata_typed(
			lua, target_index, OD_LUA_BINDINGS_RENDER_TEXTURE));
		if (!OD_CHECK(odRenderTexture_check_valid(opt_render_texture))) {
			luaL_error(lua, "odRenderTexture_check_valid() failed");    // NOTE: does not return
		}
	} else {
		luaL_error(lua, "invalid settings.target.%s=%s", OD_LUA_METATABLE_NAME_KEY, target_type);    // NOTE: does not return
	}

	OD_MAYBE_UNUSED(renderer);

	return opt_render_texture;
}
static const odTexture* odLuaBindings_odRenderer_get_src_texture_impl(lua_State* lua, int settings_index) {
	if (!OD_CHECK(lua != nullptr)) {
		return nullptr;
	}

	luaL_checktype(lua, settings_index, LUA_TTABLE);

	lua_getfield(lua, settings_index, "src");
	const int src_index = lua_gettop(lua);
	luaL_checktype(lua, src_index, LUA_TUSERDATA);

	lua_getfield(lua, src_index, OD_LUA_METATABLE_NAME_KEY);
	const char* src_type = luaL_checkstring(lua, OD_LUA_STACK_TOP);

	const odTexture* src_texture = nullptr;
	if (strcmp(src_type, OD_LUA_BINDINGS_TEXTURE) == 0) {
		src_texture = static_cast<const odTexture*>(odLua_get_userdata_typed(
			lua, src_index, OD_LUA_BINDINGS_TEXTURE));
	} else if (strcmp(src_type, OD_LUA_BINDINGS_RENDER_TEXTURE) == 0) {
		const odRenderTexture* src_render_texture = static_cast<odRenderTexture*>(odLua_get_userdata_typed(
			lua, src_index, OD_LUA_BINDINGS_RENDER_TEXTURE));
		if (!OD_CHECK(odRenderTexture_check_valid(src_render_texture))) {
			luaL_error(lua, "odRenderTexture_check_valid() failed");  // NOTE: does not return
		}

		src_texture = odRenderTexture_get_texture_const(src_render_texture);
	} else if (strcmp(src_type, OD_LUA_BINDINGS_TEXTURE_ATLAS) == 0) {
		const odTextureAtlas* src_render_texture = static_cast<odTextureAtlas*>(odLua_get_userdata_typed(
			lua, src_index, OD_LUA_BINDINGS_TEXTURE_ATLAS));
		if (!OD_CHECK(odTextureAtlas_check_valid(src_render_texture))) {
			luaL_error(lua, "odTextureAtlas_check_valid() failed");  // NOTE: does not return
		}

		src_texture = odTextureAtlas_get_texture_const(src_render_texture);
	} else {
		luaL_error(lua, "invalid settings.src.%s=%s", OD_LUA_METATABLE_NAME_KEY, src_type);  // NOTE: does not return
	}

	return src_texture;
}

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
	lua_call(lua, /*nargs*/ 0, /*nresults*/ 1);  // call metatable.default_new
	const int self_index = lua_gettop(lua);

	lua_getfield(lua, self_index, "init");
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

	lua_getfield(lua, settings_index, "color");
	const int color_index = lua_gettop(lua);
	if (!OD_CHECK(lua_type(lua, color_index) == LUA_TTABLE)) {
		return luaL_error(lua, "settings.color must be of type table");
	}

	int color_coord_count = odLua_get_length(lua, color_index);
	if (!OD_CHECK(color_coord_count == 4)) {
		return luaL_error(lua, "settings.color must have 4 elements (rgba)");
	}

	odRenderTexture* opt_render_texture = odLuaBindings_odRenderer_get_render_texture_impl(lua, settings_index, renderer);

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

	if (!OD_CHECK(odRenderer_clear(renderer, &color, opt_render_texture))) {
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
	const int render_state_index = lua_gettop(lua);
	odRenderState* render_state = static_cast<odRenderState*>(odLua_get_userdata_typed(
		lua, render_state_index, OD_LUA_BINDINGS_RENDER_STATE));
	if (!OD_CHECK(render_state != nullptr)) {
		return luaL_error(lua, "settings.render_state invalid");
	}

	lua_getfield(lua, settings_index, "vertex_array");
	const int vertex_array_index = lua_gettop(lua);
	odTrivialArrayT<odVertex>* vertex_array = static_cast<odTrivialArrayT<odVertex>*>(odLua_get_userdata_typed(
		lua, vertex_array_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return 0;
	}

	const odTexture* src_texture = odLuaBindings_odRenderer_get_src_texture_impl(lua, settings_index);
	odRenderTexture* opt_render_texture = odLuaBindings_odRenderer_get_render_texture_impl(lua, settings_index, renderer);

	if (!OD_CHECK(odRenderer_draw_vertices(renderer, vertex_array->begin(), vertex_array->get_count(), render_state, src_texture, opt_render_texture))) {
		return luaL_error(lua, "odRenderer_draw_vertices() failed");
	}

	return 0;
}
static int odLuaBindings_odRenderer_draw_texture(lua_State* lua) {
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
	const int render_state_index = lua_gettop(lua);
	odRenderState* render_state = static_cast<odRenderState*>(odLua_get_userdata_typed(
		lua, render_state_index, OD_LUA_BINDINGS_RENDER_STATE));
	if (!OD_CHECK(render_state != nullptr)) {
		return luaL_error(lua, "settings.render_state invalid");
	}

	const odTexture* src_texture = odLuaBindings_odRenderer_get_src_texture_impl(lua, settings_index);
	odRenderTexture* opt_render_texture = odLuaBindings_odRenderer_get_render_texture_impl(lua, settings_index, renderer);

	if (!OD_CHECK(odRenderer_draw_texture(renderer, render_state, src_texture, nullptr, nullptr, opt_render_texture))) {
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
		|| !OD_CHECK(add_method("draw_vertex_array", odLuaBindings_odRenderer_draw_vertex_array))
		|| !OD_CHECK(add_method("draw_texture", odLuaBindings_odRenderer_draw_texture))) {
		return false;
	}

	return true;
}
