#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// Build settings
#if !defined(OD_BUILD_DLL)
#define OD_BUILD_DLL 0
#endif

#if !defined(OD_BUILD_MODULE_ENGINE)
#define OD_BUILD_MODULE_ENGINE 0
#endif

#if !defined(OD_BUILD_LOG)
#define OD_BUILD_LOG 1
#endif

#if !defined(OD_BUILD_DEBUG_LOG)
#define OD_BUILD_DEBUG_LOG 0
#endif

#if !defined(OD_BUILD_FATAL_ASSERT)
#define OD_BUILD_FATAL_ASSERT 0
#endif

// API decorators
#if defined(__clang__) || defined(__GNUC__)
#define OD_API_PRINTF(FORMAT_ARG, VA_ARG) __attribute__((format(printf, FORMAT_ARG, VA_ARG)))
#else
#define OD_API_PRINTF(FORMAT_ARG, VA_ARG)
#endif

#if defined(__cplusplus)
#define OD_API_LINKAGE_NAMES_C extern "C"
#else
#define OD_API_LINKAGE_NAMES_C
#endif

#if OD_BUILD_DLL
#define OD_API_LINKAGE_EXPORT __declspec(dllexport)
#define OD_API_LINKAGE_IMPORT __declspec(dllimport)
#else
#define OD_API_LINKAGE_EXPORT
#define OD_API_LINKAGE_IMPORT
#endif

#if OD_BUILD_DLL && defined(OD_BUILD_MODULE_ENGINE) && OD_BUILD_MODULE_ENGINE
#define OD_API_ENGINE_LINKAGE_SHARED OD_API_LINKAGE_EXPORT
#else
#define OD_API_ENGINE_LINKAGE_SHARED OD_API_LINKAGE_IMPORT
#endif

#define OD_API_ENGINE_C OD_API_LINKAGE_NAMES_C OD_API_ENGINE_LINKAGE_SHARED 
#define OD_API_ENGINE_CPP OD_API_ENGINE_LINKAGE_SHARED 
