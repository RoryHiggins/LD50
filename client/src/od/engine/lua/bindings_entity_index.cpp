#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/vertex.h>
#include <od/core/array.hpp>
#include <od/engine/entity_index.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

static int odLuaBindings_odEntityIndex_destroy(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);

	odEntityIndex* entity_index = static_cast<odEntityIndex*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_ENTITY_INDEX));
	if (!OD_CHECK(entity_index != nullptr)) {
		return 0;
	}

	*entity_index = odEntityIndex{};

	return 0;
}
static int odLuaBindings_odEntityIndex_init(lua_State* lua) {
	return odLuaBindings_odEntityIndex_destroy(lua);
}
static int odLuaBindings_odEntityIndex_new(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int32_t metatable_index = lua_upvalueindex(1);

	luaL_checktype(lua, metatable_index, LUA_TTABLE);

	lua_getfield(lua, metatable_index, OD_LUA_DEFAULT_NEW_KEY);
	lua_call(lua, /*nargs*/ 0, /*nresults*/ 1);  // call metatable.default_new
	const int self_index = lua_gettop(lua);

	lua_getfield(lua, self_index, "init");
	lua_pushvalue(lua, self_index);
	lua_call(lua, /*nargs*/ 1, /*nresults*/ 1);

	lua_pushvalue(lua, self_index);
	return 1;
}
static int odLuaBindings_odEntityIndex_add_to_vertex_array(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	const odEntityIndex* entity_index = static_cast<odEntityIndex*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_ENTITY_INDEX));
	if (!OD_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
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

	int32_t vertex_count = 0;
	const odVertex* vertices = odEntityIndex_get_all_vertices(entity_index, &vertex_count);


	if ((vertices != nullptr) && !OD_CHECK(vertex_array->extend(vertices, vertex_count))) {
		return luaL_error(lua, "vertex_array->extend(%d) failed", vertex_count);
	}

	return 0;
}
static int odLuaBindings_odEntityIndex_set_collider(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int id_index = 2;

	const int x1_index = 3;
	const int y1_index = 4;
	const int x2_index = 5;
	const int y2_index = 6;

	const int tag_ids_start_index = 7;
	const int tag_ids_end_index = lua_gettop(lua);

	if (OD_BUILD_DEBUG) {
		luaL_checktype(lua, self_index, LUA_TUSERDATA);
		luaL_checktype(lua, id_index, LUA_TNUMBER);
		luaL_checktype(lua, x1_index, LUA_TNUMBER);
		luaL_checktype(lua, y1_index, LUA_TNUMBER);
		luaL_checktype(lua, x2_index, LUA_TNUMBER);
		luaL_checktype(lua, y2_index, LUA_TNUMBER);

		for (int i = tag_ids_start_index; i <= tag_ids_end_index; i++) {
			luaL_checktype(lua, i, LUA_TNUMBER);
		}
	}

	odEntityIndex* entity_index = static_cast<odEntityIndex*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_ENTITY_INDEX));
	if (!OD_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	odEntityCollider collider = odEntityCollider{
		static_cast<odEntityId>(lua_tonumber(lua, id_index)),
		odBounds{
			static_cast<float>(lua_tonumber(lua, x1_index)),
			static_cast<float>(lua_tonumber(lua, y1_index)),
			static_cast<float>(lua_tonumber(lua, x2_index)),
			static_cast<float>(lua_tonumber(lua, y2_index)),
		},
		odTagset{}
	};

	for (int i = tag_ids_start_index; i <= tag_ids_end_index; i++) {
		if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(lua_tonumber(lua, i))))
			|| !OD_DEBUG_CHECK(static_cast<int32_t>(lua_tonumber(lua, i)) < OD_TAG_ID_COUNT)) {
			return luaL_error(
				lua, "tag_ids[%d] must be an integer value in the range [0, %d)", i, OD_TAG_ID_COUNT);
		}

		int32_t tag_id = static_cast<int32_t>(lua_tonumber(lua, i));
		odTagset_set(&collider.tagset, tag_id, true);
	}

	if (!OD_DEBUG_CHECK(odEntityCollider_check_valid(&collider))) {
		return luaL_error(lua, "entity collider validation failed");
	}

	odEntityIndex_set_collider(entity_index, &collider);

	return 0;
}

bool odLuaBindings_odEntityIndex_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_declare(lua, OD_LUA_BINDINGS_ENTITY_INDEX))
		|| !OD_CHECK(odLua_metatable_set_new_delete(lua, OD_LUA_BINDINGS_ENTITY_INDEX, odType_get<odEntityIndex>()))) {
		return false;
	}

	auto add_method = [lua](const char* name, odLuaFn* fn) -> bool {
		return odLua_metatable_set_function(lua, OD_LUA_BINDINGS_ENTITY_INDEX, name, fn);
	};
	if (!OD_CHECK(add_method("init", odLuaBindings_odEntityIndex_init))
		|| !OD_CHECK(add_method("destroy", odLuaBindings_odEntityIndex_destroy))
		|| !OD_CHECK(add_method("new", odLuaBindings_odEntityIndex_new))
		|| !OD_CHECK(add_method("add_to_vertex_array", odLuaBindings_odEntityIndex_add_to_vertex_array))
		|| !OD_CHECK(add_method("set_collider", odLuaBindings_odEntityIndex_set_collider))) {
		return false;
	}

	return true;
}
