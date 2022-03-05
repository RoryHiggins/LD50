#include <od/engine/lua/bindings.h>

#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/platform/file.h>
#include <od/platform/image.hpp>
#include <od/platform/texture.hpp>
#include <od/platform/window.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

static bool odLuaBindings_odTexture_is_valid_operation(const char* operation) {
	if ((strcmp(operation, "init") == 0)
		|| (strcmp(operation, "init_from_png_file") == 0)) {
		return true;
	}

	return false;
}
static int odLuaBindings_odTexture_init(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;
	const int settings_index = 2;

	luaL_checktype(lua, self_index, LUA_TUSERDATA);
	luaL_checktype(lua, settings_index, LUA_TTABLE);

	odTexture* texture = static_cast<odTexture*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_TEXTURE));

	if (!OD_CHECK(texture != nullptr)
		|| !OD_CHECK(lua_type(lua, settings_index) == LUA_TTABLE)) {
		return luaL_error(lua, "odLua_get_userdata_typed(%s) failed", OD_LUA_BINDINGS_WINDOW);
	}

	lua_getfield(lua, settings_index, "window");
	const int window_index = lua_gettop(lua);
	odWindow* window = static_cast<odWindow*>(odLua_get_userdata_typed(lua, window_index, OD_LUA_BINDINGS_WINDOW));

	if (!OD_CHECK(odWindow_check_valid(window))) {
		return luaL_error(lua, "odWindow_check_valid() failed");
	}

	int32_t width = 1;
	lua_getfield(lua, settings_index, "width");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		width = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP));

		if (!OD_CHECK(width > 0)) {
			return luaL_argerror(lua, settings_index, "settings.width must be > 0");
		}
	}

	int32_t height = 1;
	lua_getfield(lua, settings_index, "height");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		height = static_cast<int32_t>(luaL_checknumber(lua, OD_LUA_STACK_TOP));

		if (!OD_CHECK(height > 0)) {
			return luaL_argerror(lua, settings_index, "settings.height must be > 0");
		}
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

	const char* operation = "init";
	lua_getfield(lua, settings_index, "operation");
	if (lua_type(lua, OD_LUA_STACK_TOP) != LUA_TNIL) {
		operation = luaL_checkstring(lua, OD_LUA_STACK_TOP);

		if (!OD_CHECK(odLuaBindings_odTexture_is_valid_operation(operation))) {
			OD_ERROR("Unknown operation=%s", operation);
			return luaL_argerror(lua, settings_index, "settings.operation not supported");
		}
	}

	odImage image{};
	const struct odColor* opt_pixels = nullptr;

	if (strcmp(operation, "init_from_png_file") == 0) {
		if (!OD_CHECK(strcmp(filename, "") != 0)
			|| !OD_CHECK(odImage_read_png_file(&image, filename))) {
			return luaL_error(lua, "odImage_read_png_file() filename=%s", filename);
		}

		opt_pixels = odImage_begin_const(&image);
		width = image.width;
		height = image.height;
	}

	if (!OD_CHECK(odTexture_init(texture, window, opt_pixels, width, height))) {
		return luaL_error(lua, "odTexture_init() failed, width=%d, height=%d", width, height);
	}

	return 0;
}
static int odLuaBindings_odTexture_destroy(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	odTexture* texture = static_cast<odTexture*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_TEXTURE));
	if (!OD_CHECK(texture != nullptr)) {
		return 0;
	}

	odTexture_destroy(texture);

	return 0;
}
static int odLuaBindings_odTexture_get_size(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return 0;
	}

	const int self_index = 1;

	odTexture* texture = static_cast<odTexture*>(odLua_get_userdata_typed(lua, self_index, OD_LUA_BINDINGS_TEXTURE));
	if (!OD_CHECK(texture != nullptr)) {
		return 0;
	}

	int32_t width = 0;
	int32_t height = 0;
	if (!OD_CHECK(odTexture_get_size(texture, &width, &height))) {
		return 0;
	}

	lua_pushnumber(lua, static_cast<lua_Number>(width));
	lua_pushnumber(lua, static_cast<lua_Number>(height));
	return 2;
}
bool odLuaBindings_odTexture_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLua_metatable_declare(lua, OD_LUA_BINDINGS_TEXTURE))
		|| !OD_CHECK(odLua_metatable_set_new_delete(lua, OD_LUA_BINDINGS_TEXTURE, odType_get<odTexture>()))) {
		return false;
	}

	auto texture_function = [lua](const char* name, odLuaFn* fn) -> bool {
		return odLua_metatable_set_function(lua, OD_LUA_BINDINGS_TEXTURE, name, fn);
	};
	if (!OD_CHECK(texture_function("init", odLuaBindings_odTexture_init))
		|| !OD_CHECK(texture_function("destroy", odLuaBindings_odTexture_destroy))
		|| !OD_CHECK(texture_function("get_size", odLuaBindings_odTexture_get_size))) {
		return false;
	}

	return true;
}
