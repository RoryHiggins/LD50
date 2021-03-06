#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Build settings
#if !defined(OD_BUILD_LUA_CLIENT)
#define OD_BUILD_LUA_CLIENT ""
#endif

#if !defined(OD_BUILD_LOGS)
#define OD_BUILD_LOGS 1
#endif

#if !defined(OD_BUILD_DEBUG)
#define OD_BUILD_DEBUG 1
#endif

#if !defined(OD_BUILD_TESTS)
#define OD_BUILD_TESTS 1
#endif

#if !defined(OD_BUILD_PROFILE)
#define OD_BUILD_PROFILE 1
#endif

#if !defined(OD_BUILD_EMSCRIPTEN)
#define OD_BUILD_EMSCRIPTEN 0
#endif

#if !defined(OD_BUILD_LUAJIT)
#define OD_BUILD_LUAJIT 0
#endif

#if !defined(OD_BUILD_LIBBACKTRACE)
#define OD_BUILD_LIBBACKTRACE 0
#endif

#if !defined(OD_BUILD_DBGHELP)
#define OD_BUILD_DBGHELP 0
#endif

// API Decorators
// More may be needed for OD_API_C on msvc, such as enforcing a cdecl calling convention
#if defined(__cplusplus)
#define OD_API_C extern "C"
#else
#define OD_API_C
#endif

#if defined(__clang__) || defined(__GNUC__)
#define OD_API_PRINTF(FORMAT_ARG, VA_ARG) __attribute__((format(printf, FORMAT_ARG, VA_ARG)))
#else
#define OD_API_PRINTF(FORMAT_ARG, VA_ARG)
#endif

#if defined(__clang__) || defined(__GNUC__)
#define OD_NO_DISCARD __attribute__((warn_unused_result))
#else
#define OD_NO_DISCARD
#endif

#define OD_UNUSED(LVALUE) static_cast<void>(LVALUE)
#define OD_MAYBE_UNUSED(LVALUE) OD_UNUSED(LVALUE)
