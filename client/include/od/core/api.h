#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Build settings
#if !defined(OD_BUILD_LOG)
#define OD_BUILD_LOG 1
#endif

#if !defined(OD_BUILD_DEBUG_LOG)
#define OD_BUILD_DEBUG_LOG 1
#endif

// Api decorators
#if defined(__clang__) || defined(__GNUC__)
#define OD_API_PRINTF(FORMAT_ARG, VA_ARG) __attribute__((format(printf, FORMAT_ARG, VA_ARG)))
#else
#define OD_API_PRINTF(FORMAT_ARG, VA_ARG)
#endif

#if defined(__cplusplus)
#define OD_API_C extern "C"
#else
#define OD_API_C
#endif
