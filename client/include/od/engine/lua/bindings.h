#pragma once

#include <od/engine/module.h>

#define OD_LUA_BINDINGS_VERTEX_ARRAY "VertexArray"
#define OD_LUA_BINDINGS_ASCII_FONT "AsciiFont"
#define OD_LUA_BINDINGS_WINDOW "Window"
#define OD_LUA_BINDINGS_TEXTURE "Texture"
#define OD_LUA_BINDINGS_RENDER_TEXTURE "RenderTexture"
#define OD_LUA_BINDINGS_RENDER_STATE "RenderState"
#define OD_LUA_BINDINGS_RENDERER "Renderer"
#define OD_LUA_BINDINGS_AUDIO "Audio"
#define OD_LUA_BINDINGS_MUSIC "Music"
#define OD_LUA_BINDINGS_TEXTURE_ATLAS "TextureAtlas"
#define OD_LUA_BINDINGS_ENTITY_INDEX "EntityIndex"

struct lua_State;

OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_odVertexArray_register(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_odAsciiFont_register(struct lua_State* lua);
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
odLuaBindings_odAudio_register(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_odMusic_register(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_odTextureAtlas_register(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_odEntityIndex_register(struct lua_State* lua);
OD_API_C OD_ENGINE_MODULE bool
odLuaBindings_register(struct lua_State* lua);
