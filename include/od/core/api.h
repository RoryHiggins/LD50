#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// Build settings
#if !defined(OD_BUILD_DLL)
#define OD_BUILD_DLL 0
#endif

#if !defined(OD_BUILD_DLL_EXPORT)
#define OD_BUILD_DLL_EXPORT 0
#endif

#if !defined(OD_BUILD_LOG)
#define OD_BUILD_LOG 1
#endif

#if !defined(OD_BUILD_DEBUG_LOG)
#define OD_BUILD_DEBUG_LOG 1
#endif

#if !defined(OD_BUILD_FATAL_ASSERT)
#define OD_BUILD_FATAL_ASSERT 0
#endif

#if !defined(OD_BUILD_DEFAULT_LOG_LEVEL)
// if unspecified, default to OD_LOG_LEVEL_WARN (2):
#define OD_BUILD_DEFAULT_LOG_LEVEL 2
#endif


// API decorators
#if defined(__clang__) || defined(__GNUC__)
#define OD_API_PRINTF(FORMAT_ARG, VA_ARG) __attribute__((format(printf, FORMAT_ARG, VA_ARG)))
#else
#define OD_API_PRINTF(FORMAT_ARG, VA_ARG)
#endif

#if OD_BUILD_DLL && OD_BUILD_DLL_EXPORT
#define OD_API_LINKAGE_SHARED __declspec(dllexport)
#elif OD_BUILD_DLL && !OD_BUILD_DLL_EXPORT
#define OD_API_LINKAGE_SHARED __declspec(dllimport)
#else
#define OD_API_LINKAGE_SHARED
#endif

#if defined(__cplusplus)
#define OD_API_LINKAGE_NAMES_C extern "C"
#else
#define OD_API_LINKAGE_NAMES_C
#endif

// TODO: do we want to explicitly set the calling convention to cdecl as well?
#define OD_API_C OD_API_LINKAGE_NAMES_C OD_API_LINKAGE_SHARED 

#define OD_API_CPP OD_API_LINKAGE_SHARED 
