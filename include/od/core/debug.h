#pragma once

#include <od/core/api.h>

#define OD_LOG_LEVEL_NONE 0
#define OD_LOG_LEVEL_ERROR 1
#define OD_LOG_LEVEL_WARN 2
#define OD_LOG_LEVEL_INFO 3
#define OD_LOG_LEVEL_DEBUG 4
#define OD_LOG_LEVEL_TRACE 5
#define OD_LOG_LEVEL_FIRST 1
#define OD_LOG_LEVEL_LAST 6

#define OD_LOG_CONTEXT_CONSTRUCT() odLogContext_construct(__FILE__, __func__, __LINE__)

#if OD_BUILD_LOG
#define OD_ERROR(...) odLog_log(OD_LOG_CONTEXT_CONSTRUCT(), OD_LOG_LEVEL_ERROR, __VA_ARGS__)
#define OD_WARN(...) odLog_log(OD_LOG_CONTEXT_CONSTRUCT(), OD_LOG_LEVEL_WARN, __VA_ARGS__)
#define OD_INFO(...) odLog_log(OD_LOG_CONTEXT_CONSTRUCT(), OD_LOG_LEVEL_INFO, __VA_ARGS__)
#define OD_ASSERT(EXPR) odLog_assert(OD_LOG_CONTEXT_CONSTRUCT(), EXPR, #EXPR)
#else
#define OD_WARN(...)
#define OD_ERROR(...)
#define OD_ASSERT(EXPR)
#endif

#if OD_BUILD_LOG && OD_BUILD_DEBUG_LOG
#define OD_DEBUG(...) odLog_log(OD_LOG_CONTEXT_CONSTRUCT(), OD_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define OD_TRACE(...) odLog_log(OD_LOG_CONTEXT_CONSTRUCT(), OD_LOG_LEVEL_TRACE, __VA_ARGS__)
#else
#define OD_TRACE(...)
#define OD_DEBUG(...)
#endif

struct odLogContext {
	const char* file;
	const char* function;
	uint32_t line;
};

OD_API_ENGINE_C const char* odLogLevel_get_name(uint32_t log_level);
OD_API_ENGINE_C uint32_t odLogLevel_get_max(void);
OD_API_ENGINE_C void odLogLevel_set_max(uint32_t log_level);

OD_API_ENGINE_C struct odLogContext odLogContext_construct(const char* file, const char* function, uint32_t line);

OD_API_ENGINE_C void odLog_log_variadic(struct odLogContext logger, uint32_t log_level, const char* format_c_str, va_list args);
OD_API_ENGINE_C void odLog_log(struct odLogContext logger, uint32_t log_level, const char* format_c_str, ...) OD_API_PRINTF(3, 4);
OD_API_ENGINE_C void odLog_assert(struct odLogContext logger, bool success, const char* expression_c_str);

OD_API_ENGINE_C char* odDebugString_allocate(uint32_t size);
OD_API_ENGINE_C const char* odDebugString_format_variadic(const char* format_c_str, va_list args);
OD_API_ENGINE_C const char* odDebugString_format(const char* format_c_str, ...) OD_API_PRINTF(1, 2);
