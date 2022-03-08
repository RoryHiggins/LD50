#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/array.hpp>
#include <od/core/vertex.h>
#include <od/platform/primitive.h>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

typedef odTrivialArrayT<odVertex> odVertexArray;

static int odLuaBindings_odVertexArray_add_vertices(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int vertices_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, vertices_index, LUA_TTABLE);

	odVertexArray* vertex_array = static_cast<odVertexArray*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_VERTEX_ARRAY);
	}

	const int elements_per_vertex_count = 10;

	const int elements_count = odLua_get_length(lua, vertices_index);
	OD_DEBUG("elements_count=%d", elements_count);
	if (!OD_DEBUG_CHECK(elements_count % (3 * elements_per_vertex_count) == 0)) {
		return luaL_error(
			lua, "settings.vertices length must be divisible by 30 (triangles of 3 xyzw+rgba+uv vertices)");
	}

	int vertices_count = elements_count / elements_per_vertex_count;
	if (!OD_CHECK(vertex_array->set_count(vertex_array->get_count() + vertices_count))) {
		return luaL_error(lua, "vertices.set_count(%d) failed", vertices_count);
	}

	odVertex* vertices_raw = vertex_array->end() - vertices_count;
	for (int i = 0; i < vertices_count; i++) {
		int vertex_offset = (i * elements_per_vertex_count) + 1;  // + 1 because lua arrays start at index 1

		lua_Number values[elements_per_vertex_count];
		for (int j = 0; j < elements_per_vertex_count; j++) {
			lua_rawgeti(lua, vertices_index, vertex_offset);

			if (!OD_DEBUG_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TNUMBER)) {
				return luaL_error(lua, "settings.vertices[%d] must be of type number", vertex_offset);
			}

			values[j] = lua_tonumber(lua, OD_LUA_STACK_TOP);
			vertex_offset++;
		}
		lua_pop(lua, elements_per_vertex_count);  // pop for every vertex so we don't hit lua stack limit

		odVertex vertex = {
			odVector{
				static_cast<float>(values[0]),
				static_cast<float>(values[1]),
				static_cast<float>(values[2]),
				static_cast<float>(values[3]),
			},
			odColor{
				static_cast<uint8_t>(values[4]),
				static_cast<uint8_t>(values[5]),
				static_cast<uint8_t>(values[6]),
				static_cast<uint8_t>(values[7]),
			},
			static_cast<float>(values[8]),
			static_cast<float>(values[9])
		};

		if (!OD_DEBUG_CHECK(odVertex_check_valid_3d(&vertex))) {
			return luaL_error(
				lua, "settings.vertices[%d-%d] not valid", i, i + 2);
		}

		for (int j = 4; j < 8; j++) {
			if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(values[j])))) {
				return luaL_error(
					lua, "settings.vertices[%d] (color) must be an integer value in the range [0, 255]", i + j);
			}
		}

		for (int j = 8; j < 10; j++) {
			if (!OD_DEBUG_CHECK(odFloat_is_precise_int24(static_cast<float>(values[j])))
				|| !OD_DEBUG_CHECK(values[j] >= 0.0)) {
				return luaL_error(
					lua, "settings.vertices[%d] (texcoord) must be an integer value in the range [0, 2^24]", i + j);
			}
		}

		vertices_raw[i] = vertex;
	}

	return 0;
}
static int odLuaBindings_odVertexArray_destroy(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odVertexArray* vertex_array = static_cast<odVertexArray*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return 0;
	}

	*vertex_array = odVertexArray{};

	return 0;
}
static int odLuaBindings_odVertexArray_init(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int vertices_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, vertices_index, LUA_TTABLE);

	odVertexArray* vertex_array = static_cast<odVertexArray*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_VERTEX_ARRAY);
	}

	*vertex_array = odVertexArray{};

	lua_getfield(lua, self_index, "add_vertices");
	if (!OD_CHECK(lua_type(lua, OD_LUA_STACK_TOP) == LUA_TFUNCTION)) {
		return luaL_error(lua, "metatable.add_vertices must be of type function");
	}

	lua_pushvalue(lua, self_index);
	lua_pushvalue(lua, vertices_index);
	lua_call(lua, /*nargs*/ 2, /*nresults*/ 1);

	return 0;
}
static int odLuaBindings_odVertexArray_new(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int vertices_index = 1;
	const int32_t metatable_index = lua_upvalueindex(1);

	luaL_checktype(lua, vertices_index, LUA_TTABLE);
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
	lua_pushvalue(lua, vertices_index);
	lua_call(lua, /*nargs*/ 2, /*nresults*/ 1);

	lua_pushvalue(lua, self_index);
	return 1;
}
static int odLuaBindings_odVertexArray_sort(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odVertexArray* vertex_array = static_cast<odVertexArray*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return 0;
	}

	*vertex_array = odVertexArray{};

	odTrianglePrimitive_sort_vertices(
		vertex_array->begin(),
		vertex_array->get_count()
	);

	return 0;
}
bool odLuaBindings_odVertexArray_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_declare(lua, OD_LUA_BINDINGS_VERTEX_ARRAY))
		|| !OD_CHECK(odLua_metatable_set_new_delete(lua, OD_LUA_BINDINGS_VERTEX_ARRAY, odType_get<odVertexArray>()))) {
		return false;
	}

	auto add_method = [lua](const char* name, odLuaFn* fn) -> bool {
		return odLua_metatable_set_function(lua, OD_LUA_BINDINGS_VERTEX_ARRAY, name, fn);
	};
	if (!OD_CHECK(add_method("init", odLuaBindings_odVertexArray_init))
		|| !OD_CHECK(add_method("new", odLuaBindings_odVertexArray_new))
		|| !OD_CHECK(add_method("destroy", odLuaBindings_odVertexArray_destroy))
		|| !OD_CHECK(add_method("sort", odLuaBindings_odVertexArray_sort))
		|| !OD_CHECK(add_method("add_vertices", odLuaBindings_odVertexArray_add_vertices))) {
		return false;
	}

	return true;
}
