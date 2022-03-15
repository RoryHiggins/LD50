#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/color.h>
#include <od/core/bounds.h>
#include <od/platform/file.h>
#include <od/platform/image.hpp>
#include <od/engine/texture_atlas.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

static int odLuaBindings_odTextureAtlas_init(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odTextureAtlas* atlas = static_cast<odTextureAtlas*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_TEXTURE_ATLAS));

	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_TEXTURE_ATLAS);
	}

	lua_getfield(lua, settings_index, "window");
	const int window_index = lua_gettop(lua);
	odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(lua, window_index, OD_LUA_BINDINGS_WINDOW));

	if (!OD_CHECK(odWindow_check_valid(window))) {
		return luaL_error(lua, "odWindow_check_valid() failed");
	}

	odImage image{};
	if (!OD_CHECK(odTextureAtlas_init(atlas, window))) {
		return luaL_error(lua, "odTextureAtlas_init() failed");
	}

	return 0;
}
static int odLuaBindings_odTextureAtlas_new(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int settings_index = 1;
	const int metatable_index = lua_upvalueindex(1);

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
static int odLuaBindings_odTextureAtlas_destroy(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	odTextureAtlas* atlas = static_cast<odTextureAtlas*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_TEXTURE_ATLAS));
	if (!OD_CHECK(atlas != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_TEXTURE_ATLAS);
	}

	odTextureAtlas_destroy(atlas);

	return 0;
}
static int odLuaBindings_odTextureAtlas_set_region_png_file(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odTextureAtlas* atlas = static_cast<odTextureAtlas*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_TEXTURE_ATLAS));

	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_TEXTURE_ATLAS);
	}

	const char* filename = "";
	lua_getfield(lua, settings_index, "filename");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		filename = luaL_checkstring(lua, OD_LUA_STACK_TOP);

		if (!OD_CHECK(odFile_get_exists(filename))) {
			OD_ERROR("Nonexistent filename=%s", filename);
			return luaL_argerror(lua, settings_index, "filename doesn't exist");
		}
	}

	lua_getfield(lua, settings_index, "id");
	const int id_index = lua_gettop(lua);
	odAtlasRegionId id = static_cast<odAtlasRegionId>(luaL_checknumber(lua, id_index));

	odImage image{};
	if (!OD_CHECK(odImage_read_png_file(&image, filename))) {
		return luaL_error(lua, "odImage_read_png_file() failed, filename=%s", filename);
	}

	int32_t width = -1;
	int32_t height = -1;
	odImage_get_size(&image, &width, &height);

	const odColor* pixels = odImage_begin_const(&image);
	if (!OD_CHECK(pixels != nullptr)) {
		return luaL_error(lua, "image pixels invalid, filename=%s", filename);
	}

	if (!OD_CHECK(odTextureAtlas_set_region(atlas, id, width, height, pixels, width))) {
		return luaL_error(lua, "odTextureAtlas_set_region() failed, filename=%s", filename);
	}

	const odBounds* bounds = odTextureAtlas_get_region_bounds(atlas, id);
	if (!OD_CHECK(odBounds_check_valid(bounds))) {
		return luaL_error(lua, "odTextureAtlas_get_region_bounds() failed");
	}

	lua_pushnumber(lua, static_cast<lua_Number>(bounds->x1));
	lua_pushnumber(lua, static_cast<lua_Number>(bounds->y1));
	lua_pushnumber(lua, static_cast<lua_Number>(bounds->x2));
	lua_pushnumber(lua, static_cast<lua_Number>(bounds->y2));
	return 4;
}
static int odLuaBindings_odTextureAtlas_reset_region(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odTextureAtlas* atlas = static_cast<odTextureAtlas*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_TEXTURE_ATLAS));

	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_TEXTURE_ATLAS);
	}

	lua_getfield(lua, settings_index, "id");
	const int id_index = lua_gettop(lua);
	odAtlasRegionId id = static_cast<odAtlasRegionId>(luaL_checknumber(lua, id_index));

	if (!OD_CHECK(odTextureAtlas_reset_region(atlas, id))) {
		return luaL_error(lua, "odTextureAtlas_reset_region() failed");
	}

	return 0;
}
static int odLuaBindings_odTextureAtlas_get_region_bounds(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odTextureAtlas* atlas = static_cast<odTextureAtlas*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_TEXTURE_ATLAS));

	if (!OD_CHECK(atlas != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_TEXTURE_ATLAS);
	}

	lua_getfield(lua, settings_index, "id");
	const int id_index = lua_gettop(lua);
	odAtlasRegionId id = static_cast<odAtlasRegionId>(luaL_checknumber(lua, id_index));

	const odBounds* bounds = odTextureAtlas_get_region_bounds(atlas, id);
	if (!OD_CHECK(odBounds_check_valid(bounds))) {
		return luaL_error(lua, "odTextureAtlas_get_region_bounds() failed");
	}

	lua_pushnumber(lua, static_cast<lua_Number>(bounds->x1));
	lua_pushnumber(lua, static_cast<lua_Number>(bounds->y1));
	lua_pushnumber(lua, static_cast<lua_Number>(bounds->x2));
	lua_pushnumber(lua, static_cast<lua_Number>(bounds->y2));
	return 4;
}
static int odLuaBindings_odTextureAtlas_get_size(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const odTextureAtlas* atlas = static_cast<odTextureAtlas*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_TEXTURE_ATLAS));
	if (!OD_CHECK(atlas != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_TEXTURE_ATLAS);
	}

	lua_pushnumber(lua, static_cast<lua_Number>(odTextureAtlas_get_width(atlas)));
	lua_pushnumber(lua, static_cast<lua_Number>(odTextureAtlas_get_height(atlas)));
	return 2;
}
static int odLuaBindings_odTextureAtlas_get_count(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	const odTextureAtlas* atlas = static_cast<odTextureAtlas*>(odLua_get_userdata_typed(
		lua, self_index, OD_LUA_BINDINGS_TEXTURE_ATLAS));
	if (!OD_CHECK(atlas != nullptr)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_TEXTURE_ATLAS);
	}

	lua_pushnumber(lua, static_cast<lua_Number>(odTextureAtlas_get_count(atlas)));
	return 1;
}
bool odLuaBindings_odTextureAtlas_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_declare(lua, OD_LUA_BINDINGS_TEXTURE_ATLAS))
		|| !OD_CHECK(odLua_metatable_set_new_delete(lua, OD_LUA_BINDINGS_TEXTURE_ATLAS, odType_get<odTextureAtlas>()))) {
		return false;
	}

	auto add_method = [lua](const char* name, odLuaFn* fn) -> bool {
		return odLua_metatable_set_function(lua, OD_LUA_BINDINGS_TEXTURE_ATLAS, name, fn);
	};
	if (!OD_CHECK(add_method("init", odLuaBindings_odTextureAtlas_init))
		|| !OD_CHECK(add_method("new", odLuaBindings_odTextureAtlas_new))
		|| !OD_CHECK(add_method("destroy", odLuaBindings_odTextureAtlas_destroy))
		|| !OD_CHECK(add_method("set_region_png_file", odLuaBindings_odTextureAtlas_set_region_png_file))
		|| !OD_CHECK(add_method("reset_region", odLuaBindings_odTextureAtlas_reset_region))
		|| !OD_CHECK(add_method("get_region_bounds", odLuaBindings_odTextureAtlas_get_region_bounds))
		|| !OD_CHECK(add_method("get_count", odLuaBindings_odTextureAtlas_get_count))
		|| !OD_CHECK(add_method("get_size", odLuaBindings_odTextureAtlas_get_size))) {
		return false;
	}

	return true;
}
