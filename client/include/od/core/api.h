#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Build settings
#if !defined(OD_BUILD_LOGS)
#define OD_BUILD_LOGS 1
#endif

#if !defined(OD_BUILD_DEBUG)
#define OD_BUILD_DEBUG 1
#endif

#if !defined(OD_BUILD_TESTS)
#define OD_BUILD_TESTS 1
#endif

#if !defined(OD_BUILD_EMSCRIPTEN)
#define OD_BUILD_EMSCRIPTEN 0
#endif

#if !defined(OD_BUILD_LIBBACKTRACE)
#define OD_BUILD_LIBBACKTRACE 0
#endif

#if !defined(OD_BUILD_DBGHELP)
#define OD_BUILD_DBGHELP 0
#endif

// Decorators
// More may need to be done here for msvc, e.g. enforcing a calling convention
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

#if defined(__cplusplus) && (__cplusplus >= 201703L)
#define OD_NO_DISCARD [[nodiscard]]
#elif defined(__clang__) || defined(__GNUC__)
#define OD_NO_DISCARD __attribute__((warn_unused_result))
#else
#define OD_NO_DISCARD
#endif

#define OD_UNUSED(LVALUE) static_cast<void>(LVALUE)
#define OD_MAYBE_UNUSED(LVALUE) OD_UNUSED(LVALUE)
