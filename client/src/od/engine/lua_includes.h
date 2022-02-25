#pragma once

#include <od/core/api.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if OD_BUILD_LUAJIT
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lualib.h>
#else
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#endif

#if defined(__cplusplus)
}  // extern "C"
#endif
