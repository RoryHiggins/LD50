#pragma once

#include <od/engine/module.h>

#define OD_LUA_BINDINGS_VERTEX_ARRAY "VertexArray"
#define OD_LUA_BINDINGS_WINDOW "Window"
#define OD_LUA_BINDINGS_TEXTURE "Texture"
#define OD_LUA_BINDINGS_RENDER_TEXTURE "RenderTexture"
#define OD_LUA_BINDINGS_RENDER_STATE "RenderState"
#define OD_LUA_BINDINGS_RENDERER "Renderer"

struct lua_State;

OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_odVertexArray_register(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_odWindow_register(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_odTexture_register(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_odRenderTexture_register(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_odRenderState_register(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_odRenderer_register(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_odEntityIndex_register(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_register(struct lua_State* lua);
