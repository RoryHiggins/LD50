#pragma once

#include <od/core/module.h>

#define OD_LOG_LEVEL_NONE 0
#define OD_LOG_LEVEL_FATAL 1
#define OD_LOG_LEVEL_ERROR 2
#define OD_LOG_LEVEL_WARN 3
#define OD_LOG_LEVEL_INFO 4
#define OD_LOG_LEVEL_DEBUG 5
#define OD_LOG_LEVEL_TRACE 6
#define OD_LOG_LEVEL_FIRST 1
#define OD_LOG_LEVEL_LAST 6
#define OD_LOG_LEVEL_DEFAULT OD_LOG_LEVEL_INFO
#define OD_LOG_GET_CONTEXT() odLogContext_init_inline(__FILE__, __func__, static_cast<int32_t>(__LINE__))
#define OD_LOG_SET_CONTEXT() odLogContext_init_temp(__FILE__, __func__, static_cast<int32_t>(__LINE__))

#if OD_BUILD_LOGS
#define OD_LOG(LEVEL, ...) odLog_log(OD_LOG_SET_CONTEXT(), LEVEL, __VA_ARGS__)
#else
#define OD_LOG(...)
#endif

#define OD_DISCARD(EXPR) odDebug_discard_result(EXPR)
#define OD_CHECK(EXPR) ((EXPR) ? true : odLog_check(OD_LOG_SET_CONTEXT(), false, #EXPR))
#define OD_ASSERT(EXPR) ((EXPR) ? true : odLog_assert(OD_LOG_SET_CONTEXT(), false, #EXPR))
#define OD_ERROR(...) OD_LOG(OD_LOG_LEVEL_ERROR, __VA_ARGS__)
#define OD_WARN(...) OD_LOG(OD_LOG_LEVEL_WARN, __VA_ARGS__)
#define OD_INFO(...) OD_LOG(OD_LOG_LEVEL_INFO, __VA_ARGS__)

#if OD_BUILD_DEBUG
#define OD_DEBUG(...) OD_LOG(OD_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define OD_TRACE(...) OD_LOG(OD_LOG_LEVEL_TRACE, __VA_ARGS__)
#define OD_DEBUG_CHECK(EXPR) OD_CHECK(EXPR)
#else
#define OD_DEBUG(...)
#define OD_TRACE(...)
#define OD_DEBUG_CHECK(EXPR) true
#endif

struct odLogContext {
	const char* file;
	const char* function;
	int32_t line;
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD void*
odDebugString_allocate(int32_t size, int32_t alignment);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odDebugString_format_variadic(const char* format_c_str, va_list args);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odDebugString_format(const char* format_c_str, ...) OD_API_PRINTF(1, 2);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odDebugString_format_array(const char* (*to_debug_str)(const void*),
						   const void* xs, int32_t count, int32_t stride);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD int32_t
odLog_get_logged_error_count(void);
OD_API_C OD_CORE_MODULE void
odLog_log_variadic(const struct odLogContext* log_context, int32_t log_level, const char* format_c_str, va_list args);
OD_API_C OD_CORE_MODULE void
odLog_log(const struct odLogContext* log_context, int32_t log_level, const char* format_c_str, ...) OD_API_PRINTF(3, 4);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD bool
odLog_check(const struct odLogContext* log_context, bool success, const char* expression_c_str);
OD_API_C OD_CORE_MODULE bool
odLog_assert(const struct odLogContext* log_context, bool success, const char* expression_c_str);

OD_API_C OD_CORE_MODULE void
odLogContext_init(struct odLogContext* log_context, const char* file, const char* function, int32_t line);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD const struct odLogContext*
odLogContext_init_temp(const char* file, const char* function, int32_t line);
inline OD_NO_DISCARD odLogContext
odLogContext_init_inline(const char* file, const char* function, int32_t line) {
	odLogContext log_context;
	odLogContext_init(&log_context, file, function, line);
	return log_context;
}

OD_API_C OD_CORE_MODULE OD_NO_DISCARD const char*
odLogLevel_get_name(int32_t log_level);
OD_API_C OD_CORE_MODULE OD_NO_DISCARD int32_t
odLogLevel_get_max(void);
OD_API_C OD_CORE_MODULE void
odLogLevel_set_max(int32_t log_level);

OD_API_C OD_CORE_MODULE void
odDebug_error(void);
OD_API_C OD_CORE_MODULE void
odDebug_set_backtrace_handler(bool(*fn)(void));
inline void odDebug_discard_result(bool result) {
	OD_MAYBE_UNUSED(result);
}
