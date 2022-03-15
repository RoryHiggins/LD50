#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/array.hpp>
#include <od/core/bounds.h>
#include <od/core/color.h>
#include <od/core/vector.h>
#include <od/core/vertex.h>
#include <od/platform/ascii_font.h>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

static int odLuaBindings_odAsciiFont_destroy(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odAsciiFont* font = static_cast<odAsciiFont*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_ASCII_FONT));
	if (!OD_CHECK(font != nullptr)) {
		return 0;
	}

	*font = odAsciiFont{};

	return 0;
}
static int odLuaBindings_odAsciiFont_init(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odAsciiFont* font = static_cast<odAsciiFont*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_ASCII_FONT));
	if (!OD_CHECK(font != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ASCII_FONT);
	}

	*font = odAsciiFont{};

	lua_getfield(lua, settings_index, "u1");
	font->texture_bounds.x1 = static_cast<float>(luaL_checknumber(lua, OD_LUA_STACK_TOP));

	lua_getfield(lua, settings_index, "v1");
	font->texture_bounds.y1 = static_cast<float>(luaL_checknumber(lua, OD_LUA_STACK_TOP));

	lua_getfield(lua, settings_index, "u2");
	font->texture_bounds.x2 = static_cast<float>(luaL_checknumber(lua, OD_LUA_STACK_TOP));

	lua_getfield(lua, settings_index, "v2");
	font->texture_bounds.y2 = static_cast<float>(luaL_checknumber(lua, OD_LUA_STACK_TOP));

	lua_getfield(lua, settings_index, "char_w");
	font->char_width = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP));

	lua_getfield(lua, settings_index, "char_h");
	font->char_height = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP));

	size_t char_first_len = 0;
	lua_getfield(lua, settings_index, "char_first");
	const char* char_first = luaL_checklstring(lua, OD_LUA_STACK_TOP, &char_first_len);
	if (!OD_CHECK(char_first != nullptr)
		|| !OD_CHECK(char_first_len == 1)) {
		return luaL_error(lua, "char_first not valid, must be a single char string");
	}
	font->char_first = char_first[0];

	size_t char_last_len = 0;
	lua_getfield(lua, settings_index, "char_last");
	const char* char_last = luaL_checklstring(lua, OD_LUA_STACK_TOP, &char_last_len);
	if (!OD_CHECK(char_last != nullptr)
		|| !OD_CHECK(char_last_len == 1)) {
		return luaL_error(lua, "char_last not valid, must be a single char string");
	}
	font->char_last = char_last[0];

	return 0;
}
static int odLuaBindings_odAsciiFont_new(lua_State* lua) {
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
static int odLuaBindings_odAsciiFont_add_text_to_vertex_array(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odAsciiFont* font = static_cast<odAsciiFont*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_ASCII_FONT));
	if (!OD_CHECK(font != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ASCII_FONT);
	}

	lua_getfield(lua, settings_index, "vertex_array");
	const int vertex_array_index = lua_gettop(lua);
	if (!OD_CHECK(lua_type(lua, vertex_array_index) == LUA_TUSERDATA)) {
		return luaL_error(lua, "settings.vertex_array must be of type userdata");
	}
	odTrivialArrayT<odVertex>* vertex_array = static_cast<odTrivialArrayT<odVertex>*>(odLua_get_userdata_typed(
		lua, vertex_array_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_VERTEX_ARRAY);
	}

	odAsciiTextPrimitive text{};

	lua_getfield(lua, settings_index, "str");
	size_t str_count = 0;
	text.str = luaL_checklstring(lua, OD_LUA_STACK_TOP, &str_count);
	text.str_count = static_cast<int32_t>(str_count);

	lua_getfield(lua, settings_index, "x");
	text.max_bounds.x1 = static_cast<float>(luaL_checknumber(lua, OD_LUA_STACK_TOP));

	lua_getfield(lua, settings_index, "y");
	text.max_bounds.y1 = static_cast<float>(luaL_checknumber(lua, OD_LUA_STACK_TOP));

	lua_getfield(lua, settings_index, "max_w");
	text.max_bounds.x2 = static_cast<float>((1 << 24) - 1);
	if (lua_type(lua, OD_LUA_STACK_TOP) == LUA_TNUMBER) {
		text.max_bounds.x2 = text.max_bounds.x1 + static_cast<float>(luaL_checknumber(lua, OD_LUA_STACK_TOP));
	}

	lua_getfield(lua, settings_index, "max_h");
	text.max_bounds.y2 = static_cast<float>((1 << 24) - 1);
	if (lua_type(lua, OD_LUA_STACK_TOP) == LUA_TNUMBER) {
		text.max_bounds.y2 = text.max_bounds.y1 + static_cast<float>(luaL_checknumber(lua, OD_LUA_STACK_TOP));
	}

	text.color = *odColor_get_white();
	lua_getfield(lua, settings_index, "color");
	const int color_index = lua_gettop(lua);
	if (lua_type(lua, color_index) == LUA_TTABLE) {
		uint8_t* color_bytes = reinterpret_cast<uint8_t*>(&text.color);
		for (int i = 1; i <= 4; i++) {
			lua_rawgeti(lua, color_index, i);
			lua_Number coord_value = lua_tonumber(lua, OD_LUA_STACK_TOP);
			if (!OD_CHECK(odFloat_is_precise_uint8(static_cast<float>(coord_value)))) {
				return luaL_error(
					lua, "settings.color[%d] must be an integer value in the range [0, 255]", i);
			}

			color_bytes[i - 1] = static_cast<uint8_t>(lua_tonumber(lua, OD_LUA_STACK_TOP));
		}
	}

	lua_getfield(lua, settings_index, "depth");
	text.depth = static_cast<float>(lua_tonumber(lua, OD_LUA_STACK_TOP));

	if (!OD_CHECK(odAsciiTextPrimitive_check_valid(&text))) {
		return luaL_error(lua, "text settings validation failed");
	}

	int32_t max_vertices_count = odAsciiTextPrimitive_get_max_vertices_count(&text);

	odTrivialArrayT<odVertex> text_vertex_array{};
	if (!OD_CHECK(text_vertex_array.set_count(max_vertices_count))) {
		return luaL_error(lua, "text_vertex_array.set_count(%d) failed", max_vertices_count);
	}
	odVertex* text_vertices_raw = text_vertex_array.begin();
	if (!OD_CHECK(text_vertices_raw != nullptr)) {
		return luaL_error(lua, "text_vertex_array.set_count(%d) failed", max_vertices_count);
	}

	int32_t vertices_count = 0;
	odBounds bounds{};
	if (!OD_CHECK(odAsciiFont_text_get_vertices(font, &text, &vertices_count, &bounds, text_vertices_raw))) {
		return luaL_error(lua, "odAsciiFont_text_get_vertices() failed");
	}

	if (!OD_CHECK(vertex_array->extend(text_vertices_raw, vertices_count))) {
		return luaL_error(lua, "vertex_array->extend() failed");
	}

	return 0;
}
bool odLuaBindings_odAsciiFont_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_declare(lua, OD_LUA_BINDINGS_ASCII_FONT))
		|| !OD_CHECK(odLua_metatable_set_new_delete(lua, OD_LUA_BINDINGS_ASCII_FONT, odType_get<odAsciiFont>()))) {
		return false;
	}

	auto add_method = [lua](const char* name, odLuaFn* fn) -> bool {
		return odLua_metatable_set_function(lua, OD_LUA_BINDINGS_ASCII_FONT, name, fn);
	};
	if (!OD_CHECK(add_method("init", odLuaBindings_odAsciiFont_init))
		|| !OD_CHECK(add_method("new", odLuaBindings_odAsciiFont_new))
		|| !OD_CHECK(add_method("destroy", odLuaBindings_odAsciiFont_destroy))
		|| !OD_CHECK(add_method("add_text_to_vertex_array", odLuaBindings_odAsciiFont_add_text_to_vertex_array))) {
		return false;
	}

	return true;
}
