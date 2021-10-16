#include <od/core/debug.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define OD_TEMP_BUFFER_CAPACITY 262144

static int32_t odLogContext_level_max = OD_LOG_LEVEL_DEFAULT;
static int32_t odLog_logged_error_count = 0;

static void od_error() {  // empty function for catching any error in a debugger
}

odLogContext odLogContext_construct(const char* file, const char* function, int32_t line) {
	if (file == nullptr) {
		file = "<file=nullptr>";
	}
	if (function == nullptr) {
		function = "<function=nullptr>";
	}

	const int32_t file_prefixes_count = 2;
	const char* file_prefixes[file_prefixes_count] = {
		"client/src/",
		"client/include/"
	};
	for (int32_t i = 0; i < file_prefixes_count; i++) {
		const char* file_relative = strstr(file, file_prefixes[i]);
		if (file_relative) {
			file = file_relative + strlen(file_prefixes[i]);
		}
	}

	return odLogContext{file, function, line};
}
void odLog_log_variadic(struct odLogContext logger, int32_t log_level, const char* format_c_str, va_list args) {
#if OD_BUILD_DEBUG
	// preconditions without assertions/logs special case here:
	// asserts can call this function, which might cause infinite recursion, so we
	// play it safe and printf
	if ((format_c_str == nullptr) || (logger.file == nullptr) || (logger.function == nullptr) ||
		((log_level < OD_LOG_LEVEL_FIRST) || (log_level > OD_LOG_LEVEL_LAST))) {

		fprintf(
			stdout,
			"odLog_log_variadic() error during logging: log_level=%d, "
			"format_c_str=%p, file=%p, line=%d, function=%p",
			log_level,
			static_cast<const void*>(format_c_str),
			static_cast<const void*>(logger.file),
			logger.line,
			static_cast<const void*>(logger.function));
		fputc('\n', stdout);
		fflush(stdout);

		od_error();
		return;
	}
#endif

	if ((log_level > odLogContext_level_max) || log_level <= OD_LOG_LEVEL_NONE) {
		return;
	}

	if (log_level <= OD_LOG_LEVEL_WARN) {
		odLog_logged_error_count++;
	}

	fprintf(stdout, "[%s %s:%d] %s() ", odLogLevel_get_name(log_level), logger.file, logger.line, logger.function);

	vfprintf(stdout, format_c_str, args);

	fputc('\n', stdout);
	fflush(stdout);

	if (log_level <= OD_LOG_LEVEL_ERROR) {
		od_error();
	}
}
void odLog_log(odLogContext logger, int32_t log_level, const char* format_c_str, ...) {
	va_list args = {};
	va_start(args, format_c_str);
	odLog_log_variadic(logger, log_level, format_c_str, args);
	va_end(args);
}
bool odLog_check(odLogContext logger, bool success, const char* expression_c_str) {
	if (!success) {
		odLog_log(logger, OD_LOG_LEVEL_ERROR, "Check failed: \"%s\"", expression_c_str);
	}
	return success;
}
void odLog_assert(odLogContext logger, bool success, const char* expression_c_str) {
	if (!success) {
		odLog_log(logger, OD_LOG_LEVEL_ERROR, "Assertion failed: \"%s\"", expression_c_str);
		exit(EXIT_FAILURE);
	}
}
int32_t odLog_get_logged_error_count() {
	return odLog_logged_error_count;
}
const char* odLogLevel_get_name(int32_t log_level) {
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
			return "<log_level_unknown>";
		}
	}
}
int32_t odLogLevel_get_max() {
	return odLogContext_level_max;
}
void odLogLevel_set_max(int32_t log_level) {
	if (log_level < OD_LOG_LEVEL_NONE) {
		OD_ERROR("log_level < OD_LOG_LEVEL_NONE");
		log_level = OD_LOG_LEVEL_NONE;
	}

	if (log_level > OD_LOG_LEVEL_LAST) {
		OD_ERROR("log_level > OD_LOG_LEVEL_LAST");
		log_level = OD_LOG_LEVEL_LAST;
	}

	odLogContext_level_max = log_level;
}

odLogLevelScoped::odLogLevelScoped() : backup_log_level{odLogLevel_get_max()} {
}
odLogLevelScoped::odLogLevelScoped(int32_t log_level) : backup_log_level{odLogLevel_get_max()} {
	odLogLevel_set_max(log_level);
}
odLogLevelScoped::~odLogLevelScoped() {
	odLogLevel_set_max(backup_log_level);
}

char* odDebugString_allocate(int32_t size) {
	const int32_t alignment = 1;
	static int32_t currentSize = 0;
	static char buffer[OD_TEMP_BUFFER_CAPACITY] = {0};

	int32_t allocated_size = size + alignment - 1;

	if (!OD_CHECK(allocated_size <= OD_TEMP_BUFFER_CAPACITY)) {
		return nullptr;
	}

	if ((currentSize + allocated_size) > OD_TEMP_BUFFER_CAPACITY) {
		currentSize = 0;
	}
	char* allocation = buffer + currentSize;
	memset(static_cast<void*>(allocation), 0, static_cast<size_t>(allocated_size));
	currentSize += allocated_size;

	uintptr_t allocation_uint = reinterpret_cast<uintptr_t>(allocation);
	uintptr_t aligned_allocation_uint = ((allocation_uint + alignment - 1) / alignment) * alignment;
	// calculate offset and add that to pointer instead of casting (performance-no-int-to-ptr)
	uintptr_t aligned_offset_uint = aligned_allocation_uint - reinterpret_cast<uintptr_t>(allocation);
	char* aligned_allocation = allocation + aligned_offset_uint;

	return aligned_allocation;
}
const char* odDebugString_format_variadic(const char* format_c_str, va_list args) {
	if (!OD_CHECK(format_c_str != nullptr)) {
		return "<format_str_null>";
	}

	va_list compute_size_args = {};
	va_copy(compute_size_args, args);
	// passing a nullptr buffer to the sprintf-family of calls will only compute
	// the output size
	int required_count = vsnprintf(/*buffer*/ nullptr, /*bufsz*/ 0, format_c_str, compute_size_args);
	va_end(compute_size_args);

	if (!OD_CHECK(required_count >= 0)) {
		return "<debug_str_format_failed>";
	}

	// sprintf-style calls always write null-terminated, but count in return value
	// excludes null terminator
	int32_t required_capacity = static_cast<int32_t>(required_count) + 1;

	void* allocation = odDebugString_allocate(required_capacity);
	if (!OD_CHECK(allocation != nullptr)) {
		return "<debug_str_allocation_failed>";
	}

	char* allocation_str = static_cast<char*>(allocation);
	vsnprintf(allocation_str, static_cast<size_t>(required_capacity), format_c_str, args);

	return allocation_str;
}
const char* odDebugString_format(const char* format_c_str, ...) {
	va_list args = {};
	va_start(args, format_c_str);
	const char* result = odDebugString_format_variadic(format_c_str, args);
	va_end(args);

	return result;
}
