#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/vertex.h>
#include <od/core/array.hpp>
#include <od/engine/tagset.h>
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
static int odLuaBindings_odEntityIndex_set(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return 0;
	}

	const int end_index = lua_gettop(lua);
	const int self_index = 1;
	const int id_index = 2;

	const int x1_index = 3;
	const int y1_index = 4;
	const int x2_index = 5;
	const int y2_index = 6;

	const int u1_index = 7;
	const int v1_index = 8;
	const int u2_index = 9;
	const int v2_index = 10;

	const int r_index = 11;
	const int g_index = 12;
	const int b_index = 13;
	const int a_index = 14;

	const int depth_index = 15;

	const int transform_start_index = 16;
	int transform_end_index = 31;

	const int tag_ids_start_index = 32;
	const int tag_ids_end_index = end_index;

	if (transform_end_index > end_index) {
		transform_end_index = end_index;
	}

	if (OD_BUILD_DEBUG) {
		luaL_checktype(lua, self_index, LUA_TUSERDATA);
		luaL_checktype(lua, id_index, LUA_TNUMBER);
		luaL_checktype(lua, x1_index, LUA_TNUMBER);
		luaL_checktype(lua, y1_index, LUA_TNUMBER);
		luaL_checktype(lua, x2_index, LUA_TNUMBER);
		luaL_checktype(lua, y2_index, LUA_TNUMBER);
		luaL_checktype(lua, u1_index, LUA_TNUMBER);
		luaL_checktype(lua, v1_index, LUA_TNUMBER);
		luaL_checktype(lua, u2_index, LUA_TNUMBER);
		luaL_checktype(lua, v2_index, LUA_TNUMBER);
		luaL_checktype(lua, r_index, LUA_TNUMBER);
		luaL_checktype(lua, g_index, LUA_TNUMBER);
		luaL_checktype(lua, b_index, LUA_TNUMBER);
		luaL_checktype(lua, a_index, LUA_TNUMBER);
		luaL_checktype(lua, depth_index, LUA_TNUMBER);

		for (int i = transform_start_index; i <= transform_end_index; i++) {
			luaL_checktype(lua, i, LUA_TNUMBER);
		}

		for (int i = tag_ids_start_index; i <= tag_ids_end_index; i++) {
			luaL_checktype(lua, i, LUA_TNUMBER);
		}
	}

	odEntityIndex* entity_index = static_cast<odEntityIndex*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_ENTITY_INDEX));
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	odEntityId entity_id = static_cast<odEntityId>(lua_tonumber(lua, id_index));
	odEntity entity = odEntity{
		odEntityCollider{
			entity_id,
			odBounds{
				static_cast<float>(lua_tonumber(lua, x1_index)),
				static_cast<float>(lua_tonumber(lua, y1_index)),
				static_cast<float>(lua_tonumber(lua, x2_index)),
				static_cast<float>(lua_tonumber(lua, y2_index)),
			},
			odTagset{}
		},
		odEntitySprite{
			odBounds{
				static_cast<float>(lua_tonumber(lua, u1_index)),
				static_cast<float>(lua_tonumber(lua, v1_index)),
				static_cast<float>(lua_tonumber(lua, u2_index)),
				static_cast<float>(lua_tonumber(lua, v2_index)),
			},
			odColor{
				static_cast<uint8_t>(lua_tonumber(lua, r_index)),
				static_cast<uint8_t>(lua_tonumber(lua, g_index)),
				static_cast<uint8_t>(lua_tonumber(lua, b_index)),
				static_cast<uint8_t>(lua_tonumber(lua, a_index)),
			},
			static_cast<float>(lua_tonumber(lua, depth_index)),
			*odMatrix_get_identity()
		}
	};

	if (OD_BUILD_DEBUG) {
		for (int i = r_index; i <= a_index; i++) {
			lua_Number coord_value = lua_tonumber(lua, i);

			if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(coord_value)))) {
				return luaL_error(
					lua, "[%d] color element must be an integer value in the range [0, 255]", i);
			}
			OD_MAYBE_UNUSED(coord_value);
		}
	}

	for (int i = transform_start_index; i <= transform_end_index; i++) {
		entity.sprite.transform.matrix[i - transform_start_index] = static_cast<float>(lua_tonumber(lua, i));
	}

	for (int i = tag_ids_start_index; i <= tag_ids_end_index; i++) {
		if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(lua_tonumber(lua, i))))
			|| !OD_DEBUG_CHECK(static_cast<int32_t>(lua_tonumber(lua, i)) < OD_TAG_ID_COUNT)) {
			return luaL_error(
				lua, "[%d] must be an integer value in the range [0, %d]", i, OD_TAG_ID_COUNT - 1);
		}

		int32_t tag_id = static_cast<int32_t>(lua_tonumber(lua, i));
		odTagset_set(&entity.collider.tagset, tag_id, true);
	}

	if (!OD_DEBUG_CHECK(odEntity_check_valid(&entity))) {
		return luaL_error(lua, "entity validation failed");
	}

	odEntityIndex_set(entity_index, &entity);

	return 0;
}
static int odLuaBindings_odEntityIndex_set_collider(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
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
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	odEntityId entity_id = static_cast<odEntityId>(lua_tonumber(lua, id_index));
	odEntityCollider collider = odEntityCollider{
		entity_id,
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
				lua, "tag_ids[%d] must be an integer value in the range [0, %d]", i, OD_TAG_ID_COUNT - 1);
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
static int odLuaBindings_odEntityIndex_set_bounds(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int id_index = 2;

	const int x1_index = 3;
	const int y1_index = 4;
	const int x2_index = 5;
	const int y2_index = 6;

	if (OD_BUILD_DEBUG) {
		luaL_checktype(lua, self_index, LUA_TUSERDATA);
		luaL_checktype(lua, id_index, LUA_TNUMBER);
		luaL_checktype(lua, x1_index, LUA_TNUMBER);
		luaL_checktype(lua, y1_index, LUA_TNUMBER);
		luaL_checktype(lua, x2_index, LUA_TNUMBER);
		luaL_checktype(lua, y2_index, LUA_TNUMBER);
	}

	odEntityIndex* entity_index = static_cast<odEntityIndex*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_ENTITY_INDEX));
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	odEntityId entity_id = static_cast<odEntityId>(lua_tonumber(lua, id_index));
	const odEntity* entity = odEntityIndex_get_or_add(entity_index, entity_id);
	if (!OD_DEBUG_CHECK(entity != nullptr)) {
		return luaL_error(lua, "odEntityIndex_get_or_add(%d) failed", entity_id);
	}

	odEntityCollider collider = entity->collider;
	collider.id = entity_id;
	collider.bounds = odBounds{
		static_cast<float>(lua_tonumber(lua, x1_index)),
		static_cast<float>(lua_tonumber(lua, y1_index)),
		static_cast<float>(lua_tonumber(lua, x2_index)),
		static_cast<float>(lua_tonumber(lua, y2_index)),
	};

	if (!OD_DEBUG_CHECK(odEntityCollider_check_valid(&collider))) {
		return luaL_error(lua, "entity collider validation failed");
	}

	odEntityIndex_set_collider(entity_index, &collider);

	return 0;
}
static int odLuaBindings_odEntityIndex_set_tags(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int id_index = 2;

	const int tag_ids_start_index = 3;
	const int tag_ids_end_index = lua_gettop(lua);

	if (OD_BUILD_DEBUG) {
		luaL_checktype(lua, self_index, LUA_TUSERDATA);
		luaL_checktype(lua, id_index, LUA_TNUMBER);

		for (int i = tag_ids_start_index; i <= tag_ids_end_index; i++) {
			luaL_checktype(lua, i, LUA_TNUMBER);
		}
	}

	odEntityIndex* entity_index = static_cast<odEntityIndex*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_ENTITY_INDEX));
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	odEntityId entity_id = static_cast<odEntityId>(lua_tonumber(lua, id_index));
	const odEntity* entity = odEntityIndex_get_or_add(entity_index, entity_id);
	if (!OD_DEBUG_CHECK(entity != nullptr)) {
		return luaL_error(lua, "odEntityIndex_get_or_add(%d) failed", entity_id);
	}

	odEntityCollider collider = entity->collider;
	collider.tagset = odTagset{};

	for (int i = tag_ids_start_index; i <= tag_ids_end_index; i++) {
		if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(lua_tonumber(lua, i))))
			|| !OD_DEBUG_CHECK(static_cast<int32_t>(lua_tonumber(lua, i)) < OD_TAG_ID_COUNT)) {
			return luaL_error(
				lua, "[%d] must be an integer value in the range [0, %d)", i, OD_TAG_ID_COUNT);
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
static int odLuaBindings_odEntityIndex_set_sprite(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return 0;
	}

	const int end_index = lua_gettop(lua);

	const int self_index = 1;

	const int id_index = 2;

	const int u1_index = 3;
	const int v1_index = 4;
	const int u2_index = 5;
	const int v2_index = 6;

	const int r_index = 7;
	const int g_index = 8;
	const int b_index = 9;
	const int a_index = 10;

	const int depth_index = 11;

	const int transform_start_index = 12;
	int transform_end_index = 27;

	if (transform_end_index > end_index) {
		transform_end_index = end_index;
	}

	if (OD_BUILD_DEBUG) {
		luaL_checktype(lua, self_index, LUA_TUSERDATA);
		luaL_checktype(lua, id_index, LUA_TNUMBER);
		luaL_checktype(lua, u1_index, LUA_TNUMBER);
		luaL_checktype(lua, v1_index, LUA_TNUMBER);
		luaL_checktype(lua, u2_index, LUA_TNUMBER);
		luaL_checktype(lua, v2_index, LUA_TNUMBER);

		if (end_index >= r_index) {
			luaL_checktype(lua, r_index, LUA_TNUMBER);
			luaL_checktype(lua, g_index, LUA_TNUMBER);
			luaL_checktype(lua, b_index, LUA_TNUMBER);
			luaL_checktype(lua, a_index, LUA_TNUMBER);
		}

		if (end_index >= depth_index) {
			luaL_checktype(lua, depth_index, LUA_TNUMBER);
		}

		for (int i = transform_start_index; i <= transform_end_index; i++) {
			luaL_checktype(lua, i, LUA_TNUMBER);
		}
	}

	odEntityId entity_id = static_cast<odEntityId>(lua_tonumber(lua, id_index));
	odEntityIndex* entity_index = static_cast<odEntityIndex*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_ENTITY_INDEX));
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	odEntitySprite sprite{
		odBounds{
			static_cast<float>(lua_tonumber(lua, u1_index)),
			static_cast<float>(lua_tonumber(lua, v1_index)),
			static_cast<float>(lua_tonumber(lua, u2_index)),
			static_cast<float>(lua_tonumber(lua, v2_index)),
		},
		*odColor_get_white(),
		static_cast<float>(lua_tonumber(lua, depth_index)),
		*odMatrix_get_identity()
	};

	if (end_index >= r_index) {
		uint8_t* color_bytes = reinterpret_cast<uint8_t*>(&sprite.color);
		for (int i = r_index; i <= a_index; i++) {
			lua_Number coord_value = lua_tonumber(lua, i);

			if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(coord_value)))) {
				return luaL_error(
					lua, "[%d] color element must be an integer value in the range [0, 255]", i);
			}

			color_bytes[i - r_index] = static_cast<uint8_t>(coord_value);
		}
	}

	for (int i = transform_start_index; i <= transform_end_index; i++) {
		sprite.transform.matrix[i - transform_start_index] = static_cast<float>(lua_tonumber(lua, i));
	}

	if (!OD_DEBUG_CHECK(odEntitySprite_check_valid(&sprite))) {
		return luaL_error(lua, "entity sprite validation failed");
	}

	odEntityIndex_set_sprite(entity_index, entity_id, &sprite);

	return 0;
}
static int odLuaBindings_odEntityIndex_get(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int id_index = 2;

	if (OD_BUILD_DEBUG) {
		luaL_checktype(lua, self_index, LUA_TUSERDATA);
		luaL_checktype(lua, id_index, LUA_TNUMBER);
	}

	odEntityId entity_id = static_cast<odEntityId>(lua_tonumber(lua, id_index));
	odEntityIndex* entity_index = static_cast<odEntityIndex*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_ENTITY_INDEX));
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	const odEntity* entity = odEntityIndex_get_or_add(entity_index, entity_id);
	if (!OD_DEBUG_CHECK(entity != nullptr)) {
		return luaL_error(lua, "odEntityIndex_get_or_add(%d) failed", entity_id);
	}

	lua_pushnumber(lua, static_cast<lua_Number>(entity->collider.bounds.x1));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->collider.bounds.y1));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->collider.bounds.x2));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->collider.bounds.y2));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.texture_bounds.x1));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.texture_bounds.y1));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.texture_bounds.x2));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.texture_bounds.y2));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.color.r));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.color.g));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.color.b));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.color.a));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.depth));
	int32_t return_count = 13;

	for (int32_t i = 0; i < OD_TAGSET_ELEMENT_COUNT; i++) {
		odTagsetElement element = entity->collider.tagset.tagset[i];
		if (element == 0) {
			continue;
		}

		int32_t i_bit = (i * OD_TAGSET_ELEMENT_BIT_SIZE);
		for (int32_t j = 0; j < OD_TAGSET_ELEMENT_BIT_SIZE; j++) {
			if ((element & (1 << j)) > 0) {
				lua_pushnumber(lua, static_cast<lua_Number>(i_bit + j));
				return_count++;
			}
		}
	}

	return return_count;
}
static int odLuaBindings_odEntityIndex_get_bounds(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int id_index = 2;

	if (OD_BUILD_DEBUG) {
		luaL_checktype(lua, self_index, LUA_TUSERDATA);
		luaL_checktype(lua, id_index, LUA_TNUMBER);
	}

	odEntityId entity_id = static_cast<odEntityId>(lua_tonumber(lua, id_index));
	odEntityIndex* entity_index = static_cast<odEntityIndex*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_ENTITY_INDEX));
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	const odEntity* entity = odEntityIndex_get_or_add(entity_index, entity_id);
	if (!OD_DEBUG_CHECK(entity != nullptr)) {
		return luaL_error(lua, "odEntityIndex_get_or_add(%d) failed", entity_id);
	}

	lua_pushnumber(lua, static_cast<lua_Number>(entity->collider.bounds.x1));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->collider.bounds.y1));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->collider.bounds.x2));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->collider.bounds.y2));
	return 4;
}
static int odLuaBindings_odEntityIndex_get_tags(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int id_index = 2;

	if (OD_BUILD_DEBUG) {
		luaL_checktype(lua, self_index, LUA_TUSERDATA);
		luaL_checktype(lua, id_index, LUA_TNUMBER);
	}

	odEntityId entity_id = static_cast<odEntityId>(lua_tonumber(lua, id_index));
	odEntityIndex* entity_index = static_cast<odEntityIndex*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_ENTITY_INDEX));
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	const odEntity* entity = odEntityIndex_get_or_add(entity_index, entity_id);
	if (!OD_DEBUG_CHECK(entity != nullptr)) {
		return luaL_error(lua, "odEntityIndex_get_or_add(%d) failed", entity_id);
	}

	int32_t return_count = 0;
	for (int32_t i = 0; i < OD_TAGSET_ELEMENT_COUNT; i++) {
		odTagsetElement element = entity->collider.tagset.tagset[i];
		if (element == 0) {
			continue;
		}

		int32_t i_bit = (i * OD_TAGSET_ELEMENT_BIT_SIZE);
		for (int32_t j = 0; j < OD_TAGSET_ELEMENT_BIT_SIZE; j++) {
			if ((element & (1 << j)) > 0) {
				lua_pushnumber(lua, static_cast<lua_Number>(i_bit + j));
				return_count++;
			}
		}
	}

	return return_count;
}
static int odLuaBindings_odEntityIndex_get_sprite(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const int id_index = 2;

	if (OD_BUILD_DEBUG) {
		luaL_checktype(lua, self_index, LUA_TUSERDATA);
		luaL_checktype(lua, id_index, LUA_TNUMBER);
	}

	odEntityId entity_id = static_cast<odEntityId>(lua_tonumber(lua, id_index));
	odEntityIndex* entity_index = static_cast<odEntityIndex*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_ENTITY_INDEX));
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	const odEntity* entity = odEntityIndex_get_or_add(entity_index, entity_id);
	if (!OD_DEBUG_CHECK(entity != nullptr)) {
		return luaL_error(lua, "odEntityIndex_get_or_add(%d) failed", entity_id);
	}

	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.texture_bounds.x1));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.texture_bounds.y1));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.texture_bounds.x2));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.texture_bounds.y2));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.color.r));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.color.g));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.color.b));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.color.a));
	lua_pushnumber(lua, static_cast<lua_Number>(entity->sprite.depth));

	return 9;
}
static int odLuaBindings_odEntityIndex_first(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int exclude_id_index = 2;
	const int x1_index = 3;
	const int y1_index = 4;
	const int x2_index = 5;
	const int y2_index = 6;

	const int tag_ids_start_index = 7;
	const int tag_ids_end_index = lua_gettop(lua);

	if (OD_BUILD_DEBUG) {
		luaL_checktype(lua, self_index, LUA_TUSERDATA);
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
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	odEntityId result;
	odEntitySearch search{
		&result,
		1,
		odBounds{
			static_cast<float>(lua_tonumber(lua, x1_index)),
			static_cast<float>(lua_tonumber(lua, y1_index)),
			static_cast<float>(lua_tonumber(lua, x2_index)),
			static_cast<float>(lua_tonumber(lua, y2_index)),
		},
		odTagset{},
		nullptr
	};

	odEntityId exclude_entity_id = 0;
	if (lua_type(lua, exclude_id_index) == LUA_TNUMBER) {
		exclude_entity_id = static_cast<odEntityId>(lua_tonumber(lua, exclude_id_index));
		search.opt_exclude_entity_id = &exclude_entity_id;
	}

	for (int i = tag_ids_start_index; i <= tag_ids_end_index; i++) {
		if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(lua_tonumber(lua, i))))
			|| !OD_DEBUG_CHECK(static_cast<int32_t>(lua_tonumber(lua, i)) < OD_TAG_ID_COUNT)) {
			return luaL_error(
				lua, "tag_ids[%d] must be an integer value in the range [0, %d]", i, OD_TAG_ID_COUNT - 1);
		}

		int32_t tag_id = static_cast<int32_t>(lua_tonumber(lua, i));
		odTagset_set(&search.tagset, tag_id, true);
	}

	if (!OD_DEBUG_CHECK(odEntitySearch_check_valid(&search))) {
		return luaL_error(lua, "entity search validation failed");
	}

	int result_count = odEntityIndex_search(entity_index, &search);
	if (result_count > 0) {
		lua_pushnumber(lua, static_cast<lua_Number>(result));
	}

	return result_count;
}
static int odLuaBindings_odEntityIndex_all(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int exclude_id_index = 2;
	const int x1_index = 3;
	const int y1_index = 4;
	const int x2_index = 5;
	const int y2_index = 6;

	const int tag_ids_start_index = 7;
	const int tag_ids_end_index = lua_gettop(lua);

	if (OD_BUILD_DEBUG) {
		luaL_checktype(lua, self_index, LUA_TUSERDATA);
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
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	static odTrivialArrayT<odEntityId> results{};
	int32_t entity_count = odEntityIndex_get_count(entity_index);

	if (entity_count == 0) {
		lua_newtable(lua);
		return 1;
	}

	if (!OD_CHECK(results.ensure_count(entity_count))) {
		return luaL_error(lua, "results.ensure_count(%d) failed", entity_count);
	}

	odEntityId* results_raw = results.begin();
	if (!OD_DEBUG_CHECK(results_raw != nullptr)) {
		return luaL_error(lua, "entity index results access or allocation failure");
	}

	odEntitySearch search{
		results_raw,
		entity_count,
		odBounds{
			static_cast<float>(lua_tonumber(lua, x1_index)),
			static_cast<float>(lua_tonumber(lua, y1_index)),
			static_cast<float>(lua_tonumber(lua, x2_index)),
			static_cast<float>(lua_tonumber(lua, y2_index)),
		},
		odTagset{},
		nullptr
	};

	odEntityId exclude_entity_id = 0;
	if (lua_type(lua, exclude_id_index) == LUA_TNUMBER) {
		exclude_entity_id = static_cast<odEntityId>(lua_tonumber(lua, exclude_id_index));
		search.opt_exclude_entity_id = &exclude_entity_id;
	}

	for (int i = tag_ids_start_index; i <= tag_ids_end_index; i++) {
		if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(lua_tonumber(lua, i))))
			|| !OD_DEBUG_CHECK(static_cast<int32_t>(lua_tonumber(lua, i)) < OD_TAG_ID_COUNT)) {
			return luaL_error(
				lua, "tag_ids[%d] must be an integer value in the range [0, %d]", i, OD_TAG_ID_COUNT - 1);
		}

		int32_t tag_id = static_cast<int32_t>(lua_tonumber(lua, i));
		odTagset_set(&search.tagset, tag_id, true);
	}

	if (!OD_DEBUG_CHECK(odEntitySearch_check_valid(&search))) {
		return luaL_error(lua, "entity search validation failed");
	}

	int result_count = odEntityIndex_search(entity_index, &search);

	lua_createtable(lua, result_count, /*nrec*/ 0);
	for (int32_t i = 0; i < result_count; i++) {
		lua_pushnumber(lua, static_cast<lua_Number>(results_raw[i]));
		lua_rawseti(lua, OD_LUA_STACK_TOP - 1, static_cast<int>(i + 1));
	}

	return 1;
}
static int odLuaBindings_odEntityIndex_count(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int exclude_id_index = 2;
	const int x1_index = 3;
	const int y1_index = 4;
	const int x2_index = 5;
	const int y2_index = 6;

	const int tag_ids_start_index = 7;
	const int tag_ids_end_index = lua_gettop(lua);

	if (OD_BUILD_DEBUG) {
		luaL_checktype(lua, self_index, LUA_TUSERDATA);
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
	if (!OD_DEBUG_CHECK(entity_index != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_ENTITY_INDEX);
	}

	int32_t entity_count = odEntityIndex_get_count(entity_index);
	odEntitySearch search{
		nullptr,
		entity_count,
		odBounds{
			static_cast<float>(lua_tonumber(lua, x1_index)),
			static_cast<float>(lua_tonumber(lua, y1_index)),
			static_cast<float>(lua_tonumber(lua, x2_index)),
			static_cast<float>(lua_tonumber(lua, y2_index)),
		},
		odTagset{},
		nullptr
	};

	odEntityId exclude_entity_id = 0;
	if (lua_type(lua, exclude_id_index) == LUA_TNUMBER) {
		exclude_entity_id = static_cast<odEntityId>(lua_tonumber(lua, exclude_id_index));
		search.opt_exclude_entity_id = &exclude_entity_id;
	}

	for (int i = tag_ids_start_index; i <= tag_ids_end_index; i++) {
		if (!OD_DEBUG_CHECK(odFloat_is_precise_uint8(static_cast<float>(lua_tonumber(lua, i))))
			|| !OD_DEBUG_CHECK(static_cast<int32_t>(lua_tonumber(lua, i)) < OD_TAG_ID_COUNT)) {
			return luaL_error(
				lua, "tag_ids[%d] must be an integer value in the range [0, %d]", i, OD_TAG_ID_COUNT - 1);
		}

		int32_t tag_id = static_cast<int32_t>(lua_tonumber(lua, i));
		odTagset_set(&search.tagset, tag_id, true);
	}

	if (!OD_DEBUG_CHECK(odEntitySearch_check_valid(&search))) {
		return luaL_error(lua, "entity search validation failed");
	}

	int result_count = odEntityIndex_search(entity_index, &search);

	lua_pushnumber(lua, static_cast<lua_Number>(result_count));
	return 1;
}
static int odLuaBindings_odEntityIndex_get_max_tag_id(lua_State* lua) {
	if (!OD_DEBUG_CHECK(lua != nullptr)) {
		return 0;
	}

	lua_pushnumber(lua, static_cast<lua_Number>(OD_TAG_ID_COUNT));
	return 1;
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
		|| !OD_CHECK(add_method("set", odLuaBindings_odEntityIndex_set))
		|| !OD_CHECK(add_method("set_collider", odLuaBindings_odEntityIndex_set_collider))
		|| !OD_CHECK(add_method("set_bounds", odLuaBindings_odEntityIndex_set_bounds))
		|| !OD_CHECK(add_method("set_tags", odLuaBindings_odEntityIndex_set_tags))
		|| !OD_CHECK(add_method("set_sprite", odLuaBindings_odEntityIndex_set_sprite))
		|| !OD_CHECK(add_method("get", odLuaBindings_odEntityIndex_get))
		|| !OD_CHECK(add_method("get_bounds", odLuaBindings_odEntityIndex_get_bounds))
		|| !OD_CHECK(add_method("get_tags", odLuaBindings_odEntityIndex_get_tags))
		|| !OD_CHECK(add_method("get_sprite", odLuaBindings_odEntityIndex_get_sprite))
		|| !OD_CHECK(add_method("first", odLuaBindings_odEntityIndex_first))
		|| !OD_CHECK(add_method("all", odLuaBindings_odEntityIndex_all))
		|| !OD_CHECK(add_method("count", odLuaBindings_odEntityIndex_count))
		|| !OD_CHECK(add_method("get_max_tag_id", odLuaBindings_odEntityIndex_get_max_tag_id))) {
		return false;
	}

	return true;
}
