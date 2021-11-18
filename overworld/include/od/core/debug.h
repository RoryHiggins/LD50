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

#define OD_LOGGER() odLogContext_construct(__FILE__, __func__, static_cast<int32_t>(__LINE__))

#define OD_CHECK(EXPR) odLog_check(OD_LOGGER(), EXPR, #EXPR)
#define OD_ASSERT(EXPR) odLog_assert(OD_LOGGER(), EXPR, #EXPR)

#if OD_BUILD_LOG
#define OD_LOG(LEVEL, ...) odLog_log(OD_LOGGER(), LEVEL, __VA_ARGS__)
#define OD_ERROR(...) OD_LOG(OD_LOG_LEVEL_ERROR, __VA_ARGS__)
#define OD_WARN(...) OD_LOG(OD_LOG_LEVEL_WARN, __VA_ARGS__)
#define OD_INFO(...) OD_LOG(OD_LOG_LEVEL_INFO, __VA_ARGS__)
#else
#define OD_LOG(...)
#define OD_ERROR(...)
#define OD_WARN(...)
#define OD_INFO(...)
#endif

#if OD_BUILD_LOG && OD_BUILD_DEBUG
#define OD_DEBUG(...) OD_LOG(OD_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define OD_TRACE(...) OD_LOG(OD_LOG_LEVEL_TRACE, __VA_ARGS__)
#else
#define OD_TRACE(...)
#define OD_DEBUG(...)
#endif

#if OD_BUILD_DEBUG
#define OD_DEBUG_ASSERT(EXPR) OD_ASSERT(EXPR)
#define OD_DEBUG_CHECK(EXPR) OD_CHECK(EXPR)
#else
#define OD_DEBUG_ASSERT(EXPR)
#define OD_DEBUG_CHECK(EXPR) true
#endif

struct odLogContext {
	const char* file;
	const char* function;
	int32_t line;
};

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const char* odLogLevel_get_name(int32_t log_level);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
int32_t odLogLevel_get_max(void);

OD_API_C OD_CORE_MODULE
void odLog_on_error(void);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
int32_t odLog_get_logged_error_count(void);

OD_API_C OD_CORE_MODULE
void odLogLevel_set_max(int32_t log_level);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
struct odLogContext odLogContext_construct(const char* file, const char* function, int32_t line);

OD_API_C OD_CORE_MODULE
void odLog_log_variadic(struct odLogContext logger, int32_t log_level, const char* format_c_str, va_list args);

OD_API_C OD_CORE_MODULE
void odLog_log(struct odLogContext logger, int32_t log_level, const char* format_c_str, ...) OD_API_PRINTF(3, 4);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
bool odLog_check(struct odLogContext logger, bool success, const char* expression_c_str);

OD_API_C OD_CORE_MODULE
void odLog_assert(struct odLogContext logger, bool success, const char* expression_c_str);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
char* odDebugString_allocate(int32_t size);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const char* odDebugString_format_variadic(const char* format_c_str, va_list args);

OD_API_C OD_CORE_MODULE OD_NO_DISCARD
const char* odDebugString_format(const char* format_c_str, ...) OD_API_PRINTF(1, 2);
