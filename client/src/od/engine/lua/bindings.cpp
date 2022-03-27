#include <od/engine/lua/bindings.h>

#include <od/core/debug.h>
#include <od/core/string.hpp>
#include <od/engine/lua/includes.h>
#include <od/engine/lua/wrappers.h>

bool odLuaBindings_register(lua_State* lua) {
	if (!OD_CHECK(lua != nullptr)) {
		return false;
	}

	if (!OD_CHECK(odLuaBindings_odVertexArray_register(lua))
		|| !OD_CHECK(odLuaBindings_odAsciiFont_register(lua))
		|| !OD_CHECK(odLuaBindings_odWindow_register(lua))
		|| !OD_CHECK(odLuaBindings_odTexture_register(lua))
		|| !OD_CHECK(odLuaBindings_odRenderTexture_register(lua))
		|| !OD_CHECK(odLuaBindings_odRenderState_register(lua))
		|| !OD_CHECK(odLuaBindings_odRenderer_register(lua))
		|| !OD_CHECK(odLuaBindings_odAudio_register(lua))
		|| !OD_CHECK(odLuaBindings_odMusic_register(lua))
		|| !OD_CHECK(odLuaBindings_odTextureAtlas_register(lua))
		|| !OD_CHECK(odLuaBindings_odEntityIndex_register(lua))) {
		return false;
	}

	return true;
}
