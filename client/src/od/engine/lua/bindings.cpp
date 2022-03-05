#include <od/engine/lua/bindings.h>

#include <od/core/debug.h>

bool odLuaBindings_odRenderTexture_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	return true;
}
bool odLuaBindings_odEntityIndex_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	return true;
}
bool odLuaBindings_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLuaBindings_odWindow_register(lua))
		|| !OD_CHECK(odLuaBindings_odTexture_register(lua))
		|| !OD_CHECK(odLuaBindings_odRenderTexture_register(lua))
		|| !OD_CHECK(odLuaBindings_odRenderState_register(lua))
		|| !OD_CHECK(odLuaBindings_odRenderer_register(lua))
		|| !OD_CHECK(odLuaBindings_odEntityIndex_register(lua))) {
		return false;
	}

	return true;
}
