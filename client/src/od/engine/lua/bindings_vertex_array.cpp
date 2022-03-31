#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/array.hpp>
#include <od/core/color.h>
#include <od/core/vector.h>
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
		return luaL_error(lua, "vertex_array->set_count(%d) failed", vertices_count);
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
static int odLuaBindings_odVertexArray_add_triangle(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int x1_index = 2;
	const int y1_index = 3;
	const int x2_index = 4;
	const int y2_index = 5;
	const int x3_index = 6;
	const int y3_index = 7;

	const int r_index = 8;
	const int g_index = 9;
	const int b_index = 10;
	const int a_index = 11;

	const int depth_index = 12;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odVertexArray* vertex_array = static_cast<odVertexArray*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_VERTEX_ARRAY);
	}

	float depth = static_cast<uint8_t>(lua_tonumber(lua, depth_index));

	odTrianglePrimitive triangle{};
	triangle.vertices[0].pos.x = static_cast<float>(luaL_checknumber(lua, x1_index));
	triangle.vertices[0].pos.y = static_cast<float>(luaL_checknumber(lua, y1_index));
	triangle.vertices[1].pos.x = static_cast<float>(luaL_checknumber(lua, x2_index));
	triangle.vertices[1].pos.y = static_cast<float>(luaL_checknumber(lua, y2_index));
	triangle.vertices[2].pos.x = static_cast<float>(luaL_checknumber(lua, x3_index));
	triangle.vertices[2].pos.y = static_cast<float>(luaL_checknumber(lua, y3_index));

	odColor color = *odColor_get_white();
	if (lua_type(lua, r_index) == LUA_TNUMBER) {
		color.r = static_cast<uint8_t>(lua_tonumber(lua, r_index));
	}
	if (lua_type(lua, g_index) == LUA_TNUMBER) {
		color.g = static_cast<uint8_t>(lua_tonumber(lua, g_index));
	}
	if (lua_type(lua, b_index) == LUA_TNUMBER) {
		color.b = static_cast<uint8_t>(lua_tonumber(lua, b_index));
	}
	if (lua_type(lua, a_index) == LUA_TNUMBER) {
		color.a = static_cast<uint8_t>(lua_tonumber(lua, a_index));
	}

	for (int i = r_index; i <= a_index; i++) {
		if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(luaL_checknumber(lua, i))))) {
			return luaL_error(
				lua, "args[%d] (color element) must be an integer value in the range [0, 255]", i);
		}
	}

	const int32_t vertex_count = OD_TRIANGLE_VERTEX_COUNT;
	for (int32_t i = 0; i < vertex_count; i++) {
		triangle.vertices[i].color = color;
		triangle.vertices[i].pos.z = depth;
		triangle.vertices[i].pos.w = 1.0f;

		if (!OD_DEBUG_CHECK(odVertex_check_valid_3d(&triangle.vertices[i]))) {
			return luaL_error(lua, "vertex[%d] not valid", i);
		}
	}

	if (!OD_CHECK(vertex_array->extend(triangle.vertices, vertex_count))) {
		return luaL_error(lua, "vertex_array->extend(%s) failed", vertex_count);
	}

	return 0;
}
static int odLuaBindings_odVertexArray_add_sprite(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int x1_index = 2;
	const int y1_index = 3;
	const int x2_index = 4;
	const int y2_index = 5;

	const int u1_index = 6;
	const int v1_index = 7;
	const int u2_index = 8;
	const int v2_index = 9;

	const int r_index = 10;
	const int g_index = 11;
	const int b_index = 12;
	const int a_index = 13;

	const int depth_index = 14;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odVertexArray* vertex_array = static_cast<odVertexArray*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_VERTEX_ARRAY);
	}

	odSpritePrimitive sprite{};
	sprite.bounds.x1 = static_cast<float>(luaL_checknumber(lua, x1_index));
	sprite.bounds.y1 = static_cast<float>(luaL_checknumber(lua, y1_index));
	sprite.bounds.x2 = static_cast<float>(luaL_checknumber(lua, x2_index));
	sprite.bounds.y2 = static_cast<float>(luaL_checknumber(lua, y2_index));

	sprite.texture_bounds.x1 = static_cast<float>(luaL_checknumber(lua, u1_index));
	sprite.texture_bounds.y1 = static_cast<float>(luaL_checknumber(lua, v1_index));
	sprite.texture_bounds.x2 = static_cast<float>(luaL_checknumber(lua, u2_index));
	sprite.texture_bounds.y2 = static_cast<float>(luaL_checknumber(lua, v2_index));

	sprite.color = *odColor_get_white();
	if (lua_type(lua, r_index) == LUA_TNUMBER) {
		sprite.color.r = static_cast<uint8_t>(lua_tonumber(lua, r_index));
	}
	if (lua_type(lua, g_index) == LUA_TNUMBER) {
		sprite.color.g = static_cast<uint8_t>(lua_tonumber(lua, g_index));
	}
	if (lua_type(lua, b_index) == LUA_TNUMBER) {
		sprite.color.b = static_cast<uint8_t>(lua_tonumber(lua, b_index));
	}
	if (lua_type(lua, a_index) == LUA_TNUMBER) {
		sprite.color.a = static_cast<uint8_t>(lua_tonumber(lua, a_index));
	}

	sprite.depth = static_cast<uint8_t>(lua_tonumber(lua, depth_index));

	for (int i = r_index; i <= a_index; i++) {
		if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(luaL_checknumber(lua, i))))) {
			return luaL_error(
				lua, "args[%d] (color element) must be an integer value in the range [0, 255]", i);
		}
	}

	if (!OD_DEBUG_CHECK(odSpritePrimitive_check_valid(&sprite))) {
		return luaL_error(lua, "vertex validation failed");
	}

	const int32_t vertex_count = OD_SPRITE_VERTEX_COUNT;
	odVertex vertices[vertex_count];
	odSpritePrimitive_get_vertices(&sprite, vertices);

	if (!OD_CHECK(vertex_array->extend(vertices, vertex_count))) {
		return luaL_error(lua, "vertex_array->extend(%s) failed", vertex_count);
	}

	return 0;
}
static int odLuaBindings_odVertexArray_add_rect(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int x1_index = 2;
	const int y1_index = 3;
	const int x2_index = 4;
	const int y2_index = 5;

	const int r_index = 6;
	const int g_index = 7;
	const int b_index = 8;
	const int a_index = 9;

	const int depth_index = 10;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odVertexArray* vertex_array = static_cast<odVertexArray*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_VERTEX_ARRAY);
	}

	odSpritePrimitive sprite{};
	sprite.bounds.x1 = static_cast<float>(luaL_checknumber(lua, x1_index));
	sprite.bounds.y1 = static_cast<float>(luaL_checknumber(lua, y1_index));
	sprite.bounds.x2 = static_cast<float>(luaL_checknumber(lua, x2_index));
	sprite.bounds.y2 = static_cast<float>(luaL_checknumber(lua, y2_index));

	sprite.color = *odColor_get_white();
	if (lua_type(lua, r_index) == LUA_TNUMBER) {
		sprite.color.r = static_cast<uint8_t>(lua_tonumber(lua, r_index));
	}
	if (lua_type(lua, g_index) == LUA_TNUMBER) {
		sprite.color.g = static_cast<uint8_t>(lua_tonumber(lua, g_index));
	}
	if (lua_type(lua, b_index) == LUA_TNUMBER) {
		sprite.color.b = static_cast<uint8_t>(lua_tonumber(lua, b_index));
	}
	if (lua_type(lua, a_index) == LUA_TNUMBER) {
		sprite.color.a = static_cast<uint8_t>(lua_tonumber(lua, a_index));
	}

	sprite.depth = static_cast<uint8_t>(lua_tonumber(lua, depth_index));

	for (int i = r_index; i <= a_index; i++) {
		if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(luaL_checknumber(lua, i))))) {
			return luaL_error(
				lua, "args[%d] (color element) must be an integer value in the range [0, 255]", i);
		}
	}

	if (!OD_DEBUG_CHECK(odSpritePrimitive_check_valid(&sprite))) {
		return luaL_error(lua, "vertex validation failed");
	}

	const int32_t vertex_count = OD_SPRITE_VERTEX_COUNT;
	odVertex vertices[vertex_count];
	odSpritePrimitive_get_vertices(&sprite, vertices);

	if (!OD_CHECK(vertex_array->extend(vertices, vertex_count))) {
		return luaL_error(lua, "vertex_array->extend(%s) failed", vertex_count);
	}

	return 0;
}
static int odLuaBindings_odVertexArray_add_rect_outline(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int x1_index = 2;
	const int y1_index = 3;
	const int x2_index = 4;
	const int y2_index = 5;

	const int r_index = 6;
	const int g_index = 7;
	const int b_index = 8;
	const int a_index = 9;

	const int depth_index = 10;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odVertexArray* vertex_array = static_cast<odVertexArray*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_VERTEX_ARRAY);
	}

	odLinePrimitive line{};

	line.color = *odColor_get_white();
	if (lua_type(lua, r_index) == LUA_TNUMBER) {
		line.color.r = static_cast<uint8_t>(lua_tonumber(lua, r_index));
	}
	if (lua_type(lua, g_index) == LUA_TNUMBER) {
		line.color.g = static_cast<uint8_t>(lua_tonumber(lua, g_index));
	}
	if (lua_type(lua, b_index) == LUA_TNUMBER) {
		line.color.b = static_cast<uint8_t>(lua_tonumber(lua, b_index));
	}
	if (lua_type(lua, a_index) == LUA_TNUMBER) {
		line.color.a = static_cast<uint8_t>(lua_tonumber(lua, a_index));
	}

	line.depth = static_cast<uint8_t>(lua_tonumber(lua, depth_index));

	for (int i = r_index; i <= a_index; i++) {
		if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(luaL_checknumber(lua, i))))) {
			return luaL_error(
				lua, "args[%d] (color element) must be an integer value in the range [0, 255]", i);
		}
	}

	if (!OD_DEBUG_CHECK(odLinePrimitive_check_valid(&line))) {
		return luaL_error(lua, "line validation failed");
	}

	const int32_t vertex_count = 4 * OD_LINE_VERTEX_COUNT;
	odVertex vertices[vertex_count];

	// top
	line.bounds.x1 = static_cast<float>(luaL_checknumber(lua, x1_index));
	line.bounds.y1 = static_cast<float>(luaL_checknumber(lua, y1_index));
	line.bounds.x2 = static_cast<float>(luaL_checknumber(lua, x2_index));
	line.bounds.y2 = static_cast<float>(luaL_checknumber(lua, y1_index));
	odLinePrimitive_get_vertices(&line, vertices + (0 * OD_LINE_VERTEX_COUNT));

	// bottom
	line.bounds.x1 = static_cast<float>(luaL_checknumber(lua, x1_index));
	line.bounds.y1 = static_cast<float>(luaL_checknumber(lua, y2_index) - 1.0);
	line.bounds.x2 = static_cast<float>(luaL_checknumber(lua, x2_index));
	line.bounds.y2 = static_cast<float>(luaL_checknumber(lua, y2_index) - 1.0);
	odLinePrimitive_get_vertices(&line, vertices + (1 * OD_LINE_VERTEX_COUNT));

	// left
	line.bounds.x1 = static_cast<float>(luaL_checknumber(lua, x1_index));
	line.bounds.y1 = static_cast<float>(luaL_checknumber(lua, y1_index) + 1.0);
	line.bounds.x2 = static_cast<float>(luaL_checknumber(lua, x1_index));
	line.bounds.y2 = static_cast<float>(luaL_checknumber(lua, y2_index) - 1.0);
	odLinePrimitive_get_vertices(&line, vertices + (2 * OD_LINE_VERTEX_COUNT));

	// right
	line.bounds.x1 = static_cast<float>(luaL_checknumber(lua, x2_index) - 1.0);
	line.bounds.y1 = static_cast<float>(luaL_checknumber(lua, y1_index) + 1.0);
	line.bounds.x2 = static_cast<float>(luaL_checknumber(lua, x2_index) - 1.0);
	line.bounds.y2 = static_cast<float>(luaL_checknumber(lua, y2_index) - 1.0);
	odLinePrimitive_get_vertices(&line, vertices + (3 * OD_LINE_VERTEX_COUNT));

	if (!OD_CHECK(vertex_array->extend(vertices, vertex_count))) {
		return luaL_error(lua, "vertex_array->extend(%s) failed", vertex_count);
	}

	return 0;
}
static int odLuaBindings_odVertexArray_add_line(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int x1_index = 2;
	const int y1_index = 3;
	const int x2_index = 4;
	const int y2_index = 5;

	const int r_index = 6;
	const int g_index = 7;
	const int b_index = 8;
	const int a_index = 9;

	const int depth_index = 10;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odVertexArray* vertex_array = static_cast<odVertexArray*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_VERTEX_ARRAY);
	}

	odLinePrimitive line{};
	line.bounds.x1 = static_cast<float>(luaL_checknumber(lua, x1_index));
	line.bounds.y1 = static_cast<float>(luaL_checknumber(lua, y1_index));
	line.bounds.x2 = static_cast<float>(luaL_checknumber(lua, x2_index));
	line.bounds.y2 = static_cast<float>(luaL_checknumber(lua, y2_index));

	line.color = *odColor_get_white();
	if (lua_type(lua, r_index) == LUA_TNUMBER) {
		line.color.r = static_cast<uint8_t>(lua_tonumber(lua, r_index));
	}
	if (lua_type(lua, g_index) == LUA_TNUMBER) {
		line.color.g = static_cast<uint8_t>(lua_tonumber(lua, g_index));
	}
	if (lua_type(lua, b_index) == LUA_TNUMBER) {
		line.color.b = static_cast<uint8_t>(lua_tonumber(lua, b_index));
	}
	if (lua_type(lua, a_index) == LUA_TNUMBER) {
		line.color.a = static_cast<uint8_t>(lua_tonumber(lua, a_index));
	}

	line.depth = static_cast<uint8_t>(lua_tonumber(lua, depth_index));

	for (int i = r_index; i <= a_index; i++) {
		if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(luaL_checknumber(lua, i))))) {
			return luaL_error(
				lua, "args[%d] (color element) must be an integer value in the range [0, 255]", i);
		}
	}

	if (!OD_DEBUG_CHECK(odLinePrimitive_check_valid(&line))) {
		return luaL_error(lua, "line validation failed");
	}

	const int32_t vertex_count = OD_LINE_VERTEX_COUNT;
	odVertex vertices[vertex_count];
	odLinePrimitive_get_vertices(&line, vertices);

	if (!OD_CHECK(vertex_array->extend(vertices, vertex_count))) {
		return luaL_error(lua, "vertex_array->extend(%s) failed", vertex_count);
	}

	return 0;
}
static int odLuaBindings_odVertexArray_add_point(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int x_index = 2;
	const int y_index = 3;

	const int r_index = 4;
	const int g_index = 5;
	const int b_index = 6;
	const int a_index = 7;

	const int depth_index = 7;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odVertexArray* vertex_array = static_cast<odVertexArray*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_VERTEX_ARRAY));
	if (!OD_CHECK(vertex_array != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_VERTEX_ARRAY);
	}

	odSpritePrimitive sprite{};
	sprite.bounds.x1 = static_cast<float>(luaL_checknumber(lua, x_index));
	sprite.bounds.y1 = static_cast<float>(luaL_checknumber(lua, y_index));
	sprite.bounds.x2 = static_cast<float>(luaL_checknumber(lua, x_index) + 1.0);
	sprite.bounds.y2 = static_cast<float>(luaL_checknumber(lua, y_index) + 1.0);

	sprite.color = *odColor_get_white();
	if (lua_type(lua, r_index) == LUA_TNUMBER) {
		sprite.color.r = static_cast<uint8_t>(lua_tonumber(lua, r_index));
	}
	if (lua_type(lua, g_index) == LUA_TNUMBER) {
		sprite.color.g = static_cast<uint8_t>(lua_tonumber(lua, g_index));
	}
	if (lua_type(lua, b_index) == LUA_TNUMBER) {
		sprite.color.b = static_cast<uint8_t>(lua_tonumber(lua, b_index));
	}
	if (lua_type(lua, a_index) == LUA_TNUMBER) {
		sprite.color.a = static_cast<uint8_t>(lua_tonumber(lua, a_index));
	}

	sprite.depth = static_cast<uint8_t>(lua_tonumber(lua, depth_index));

	for (int i = r_index; i <= a_index; i++) {
		if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(luaL_checknumber(lua, i))))) {
			return luaL_error(
				lua, "args[%d] (color element) must be an integer value in the range [0, 255]", i);
		}
	}

	if (!OD_DEBUG_CHECK(odSpritePrimitive_check_valid(&sprite))) {
		return luaL_error(lua, "vertex validation failed");
	}

	const int32_t vertex_count = OD_SPRITE_VERTEX_COUNT;
	odVertex vertices[vertex_count];
	odSpritePrimitive_get_vertices(&sprite, vertices);

	if (!OD_CHECK(vertex_array->extend(vertices, vertex_count))) {
		return luaL_error(lua, "vertex_array->extend(%s) failed", vertex_count);
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
	lua_call(lua, /*nargs*/ 0, /*nresults*/ 1);  // call metatable.default_new
	const int self_index = lua_gettop(lua);

	lua_getfield(lua, self_index, "init");
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
		|| !OD_CHECK(add_method("add_vertices", odLuaBindings_odVertexArray_add_vertices))
		|| !OD_CHECK(add_method("add_triangle", odLuaBindings_odVertexArray_add_triangle))
		|| !OD_CHECK(add_method("add_sprite", odLuaBindings_odVertexArray_add_sprite))
		|| !OD_CHECK(add_method("add_rect", odLuaBindings_odVertexArray_add_rect))
		|| !OD_CHECK(add_method("add_rect_outline", odLuaBindings_odVertexArray_add_rect_outline))
		|| !OD_CHECK(add_method("add_line", odLuaBindings_odVertexArray_add_line))
		|| !OD_CHECK(add_method("add_point", odLuaBindings_odVertexArray_add_point))) {
		return false;
	}

	return true;
}
