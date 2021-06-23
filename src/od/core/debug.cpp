#include <od/core/debug.hpp>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define OD_TEMP_BUFFER_CAPACITY 262144

static uint32_t odLogContextLevelMax = OD_LOG_LEVEL_DEFAULT;

static void od_error() {  // empty function for catching any error in a debugger
}

odLogContext odLogContext_construct(const char* file, const char* function, uint32_t line) {
	if (file == nullptr) {
		file = "<file=nullptr>";
	}
	if (function == nullptr) {
		function = "<function=nullptr>";
	}

	return odLogContext{file, function, line};
}
void odLog_log_variadic(struct odLogContext logger, uint32_t log_level, const char* format_c_str, va_list args) {
#if OD_BUILD_DEBUG_LOG
	// preconditions without assertions/logs special case here:
	// asserts can call this function, which might cause infinite recursion, so we play it safe and printf
	if ((format_c_str == nullptr)
		|| (logger.file == nullptr)
		|| (logger.function == nullptr)
		|| ((log_level < OD_LOG_LEVEL_FIRST) || (log_level > OD_LOG_LEVEL_LAST))) {

		fprintf(
			stdout,
			"odLog_log_variadic() error during logging: log_level=%u, format_c_str=%p, file=%p, line=%u, function=%p",
			log_level,
			static_cast<const void*>(format_c_str),
			static_cast<const void*>(logger.file),
			logger.line,
			static_cast<const void*>(logger.function)
		);
		fputc('\n', stdout);
		fflush(stdout);

		od_error();
		return;
	}
#endif

	if (log_level > odLogContextLevelMax) {
		return;
	}

	fprintf(
		stdout,
		"[%s %s:%u] %s() ",
		odLogLevel_get_name(log_level),
		logger.file,
		logger.line,
		logger.function
	);

	vfprintf(stdout, format_c_str, args);

	fputc('\n', stdout);
	fflush(stdout);

	if (log_level <= OD_LOG_LEVEL_ERROR) {
		od_error();
	}
}
void odLog_log(odLogContext logger, uint32_t log_level, const char* format_c_str, ...) {
	va_list args;
	va_start(args, format_c_str);
	odLog_log_variadic(logger, log_level, format_c_str, args);
	va_end(args);
}
void odLog_assert(odLogContext logger, bool success, const char* expression_c_str) {
	if (!success) {
		odLog_log(logger, OD_LOG_LEVEL_ERROR, "Assertion failed: \"%s\"", expression_c_str);
		exit(EXIT_FAILURE);
	}
}
const char* odLogLevel_get_name(uint32_t log_level) {
	switch (log_level) {
		case OD_LOG_LEVEL_NONE: {
			return "NONE ";
		}
		case OD_LOG_LEVEL_ERROR: {
			return "ERROR";
		}
		case OD_LOG_LEVEL_WARN: {
			return "WARN ";
		}
		case OD_LOG_LEVEL_INFO: {
			return "info ";
		}
		case OD_LOG_LEVEL_DEBUG: {
			return "debug";
		}
		case OD_LOG_LEVEL_TRACE: {
			return "trace";
		}
		default: {
			OD_ERROR("unknown log level, log_level=%u", log_level);
			return "<unknown log level>";
		}
	}
}
uint32_t odLogLevel_get_max(void) {
	return odLogContextLevelMax;
}
void odLogLevel_set_max(uint32_t log_level) {
	odLogContextLevelMax = log_level;
}

odLogLevelScoped::odLogLevelScoped()
: backup_log_level{odLogLevel_get_max()} {
}
odLogLevelScoped::odLogLevelScoped(uint32_t log_level)
: backup_log_level{odLogLevel_get_max()} {
	odLogLevel_set_max(log_level);
}
odLogLevelScoped::~odLogLevelScoped() {
	odLogLevel_set_max(backup_log_level);
}

char* odDebugString_allocate(uint32_t size) {
	const uint32_t alignment = 1;
	static uint32_t currentSize = 0;
	static char buffer[OD_TEMP_BUFFER_CAPACITY] = {0};

	uint32_t allocated_size = size + alignment - 1;

	if (allocated_size > OD_TEMP_BUFFER_CAPACITY) {
		OD_ERROR("insufficient capacity, allocated_size=%u", allocated_size);
		return nullptr;
	}

	if ((currentSize + allocated_size) > OD_TEMP_BUFFER_CAPACITY) {
		currentSize = 0;
	}
	char* allocation = buffer + currentSize;
	memset(static_cast<void*>(allocation), 0, allocated_size);
	currentSize += allocated_size;

	uintptr_t allocation_uint = reinterpret_cast<uintptr_t>(allocation);
	uintptr_t aligned_allocation_uint = ((allocation_uint + alignment - 1) / alignment) * alignment;
	char* aligned_allocation = reinterpret_cast<char*>(aligned_allocation_uint);

	return aligned_allocation;
}
const char* odDebugString_format_variadic(const char* format_c_str, va_list args) {
	if (format_c_str == nullptr) {
		OD_ERROR("format_c_str=nullptr");
		return "<format_c_str=nullptr>";
	}

	va_list compute_size_args;
	va_copy(compute_size_args, args);
	// passing a nullptr buffer to the sprintf-family of calls will only compute the output size
	int required_count = vsnprintf(/*buffer*/ nullptr, /*bufsz*/ 0, format_c_str, compute_size_args);
	va_end(compute_size_args);

	if (required_count < 0) {
		OD_ERROR("failed to parse debug format string, format_c_str=%s", format_c_str);
		return "<failed to parse debug format string>";
	}

	// sprintf-style calls always write null-terminated, but count in return value excludes null terminator
	uint32_t required_capacity = static_cast<uint32_t>(required_count) + 1;

	void* allocation = odDebugString_allocate(required_capacity);
	if (allocation == nullptr) {
		OD_ERROR("failed to allocate debug string, format_c_str=%s, required_capacity=%u", format_c_str, required_capacity);
		return "<failed to allocate debug string>";
	}

	char* allocation_str = static_cast<char*>(allocation);
	vsnprintf(allocation_str, static_cast<size_t>(required_capacity), format_c_str, args);

	return allocation_str;
}
const char* odDebugString_format(const char* format_c_str, ...) {
	va_list args;
	va_start(args, format_c_str);
	const char* result = odDebugString_format_variadic(format_c_str, args);
	va_end(args);

	return result;
}
