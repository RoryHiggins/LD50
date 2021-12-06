#include <od/core/debug.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

// we intentionally don't mix streams, to avoid out-of-order output
#define OD_DEBUG_OUT_STREAM stdout

#define OD_TEMP_BUFFER_CAPACITY 262144

static bool (*odDebug_platform_backtrace_handler)() = nullptr;

void odPlatformDebug_set_backtrace_handler(bool(*handler)()) {
	odDebug_platform_backtrace_handler = handler;
}

void odDebug_error() {
	if (odDebug_platform_backtrace_handler != nullptr) {
		odDebug_platform_backtrace_handler();
	}
}
static int32_t odLogContext_level_max = OD_LOG_LEVEL_DEFAULT;
static int32_t odLog_logged_error_count = 0;

void odLogContext_init(struct odLogContext* log_context, const char* file, const char* function, int32_t line) {
	if (log_context == nullptr) {
		// using our own logging/assertion code here would risking an infinite loop
		fprintf(OD_DEBUG_OUT_STREAM, "error initializing log context");
		fflush(OD_DEBUG_OUT_STREAM);
		exit(EXIT_FAILURE);
	}

	if (OD_BUILD_DEBUG) {
		if (file == nullptr) {
			file = "\"<file=nullptr>\"";
		}
		if (function == nullptr) {
			function = "\"<function=nullptr>\"";
		}
	}

	*log_context = odLogContext{file, function, line};
}
const odLogContext* odLogContext_init_temp(const char* file, const char* function, int32_t line) {
	static odLogContext log_context{};
	log_context = odLogContext_init_inline(file, function, line);

	return &log_context;
}

const char* odLogLevel_get_name(int32_t log_level) {
	switch (log_level) {
		case OD_LOG_LEVEL_FATAL: {
			return "FATAL";
		}
		case OD_LOG_LEVEL_ERROR: {
			return "ERROR";
		}
		case OD_LOG_LEVEL_WARN: {
			return "WARN";
		}
		case OD_LOG_LEVEL_INFO: {
			return "info";
		}
		case OD_LOG_LEVEL_DEBUG: {
			return "debug";
		}
		case OD_LOG_LEVEL_TRACE: {
			return "trace";
		}
		default: {
			return "\"<log_level_unknown>\"";
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

static const char* odLog_get_short_filename(const char* file) {
	const int32_t file_prefixes_count = 4;
	const char* file_prefixes[file_prefixes_count] = {
		"client/src/od/",
		"client/include/od/",
		"client\\src\\od\\",
		"client\\include\\od\\"
	};

	for (int32_t i = 0; i < file_prefixes_count; i++) {
		const char* file_relative = strstr(file, file_prefixes[i]);
		if (file_relative) {
			file = file_relative + strlen(file_prefixes[i]);
		}
	}

	return file;
}
void odLog_log_variadic(const struct odLogContext* log_context, int32_t log_level, const char* format_c_str, va_list args) {
	if (OD_BUILD_DEBUG) {
		// preconditions without assertions/logs special case here:
		// asserts can call this function, which might cause infinite recursion, so we
		// play it safe and printf
		if ((format_c_str == nullptr) || (log_context->file == nullptr) || (log_context->function == nullptr) ||
			((log_level < OD_LOG_LEVEL_FIRST) || (log_level > OD_LOG_LEVEL_LAST))) {
			odDebug_error();

			fprintf(
				OD_DEBUG_OUT_STREAM,
				"odLog_log_variadic() error during logging: log_level=%d, "
				"format_c_str=%p, file=%p, line=%d, function=%p",
				log_level,
				static_cast<const void*>(format_c_str),
				static_cast<const void*>(log_context->file),
				log_context->line,
				static_cast<const void*>(log_context->function));
			fputc('\n', OD_DEBUG_OUT_STREAM);
			fflush(OD_DEBUG_OUT_STREAM);

			return;
		}
	}

	if ((log_level > odLogContext_level_max) || (log_level <= OD_LOG_LEVEL_NONE)) {
		return;
	}

	if (log_level <= OD_LOG_LEVEL_WARN) {
		odLog_logged_error_count++;
	}

	time_t time_val = time(nullptr);
    fprintf(OD_DEBUG_OUT_STREAM, "[%.8s %s %s:%d %s] ", ctime(&time_val) + 11, odLogLevel_get_name(log_level), odLog_get_short_filename(log_context->file), log_context->line, log_context->function);

	vfprintf(OD_DEBUG_OUT_STREAM, format_c_str, args);

	fputc('\n', OD_DEBUG_OUT_STREAM);
	fflush(OD_DEBUG_OUT_STREAM);

	if (OD_BUILD_DEBUG && (log_level <= OD_LOG_LEVEL_ERROR)) {
		odDebug_error();
	}
}
void odLog_log(const struct odLogContext* log_context, int32_t log_level, const char* format_c_str, ...) {
	va_list args = {};
	va_start(args, format_c_str);
	odLog_log_variadic(log_context, log_level, format_c_str, args);
	va_end(args);
}
bool odLog_check(const struct odLogContext* log_context, bool success, const char* expression_c_str) {
	if (!success) {
		odLog_log(log_context, OD_LOG_LEVEL_ERROR, "Check failed: \"%s\"", expression_c_str);
	}
	return success;
}
bool odLog_assert(const struct odLogContext* log_context, bool success, const char* expression_c_str) {
	if (!success) {
		odLog_log(log_context, OD_LOG_LEVEL_FATAL, "Assertion failed: \"%s\"", expression_c_str);
		exit(EXIT_FAILURE);
	}
	return success;
}
int32_t odLog_get_logged_error_count() {
	return odLog_logged_error_count;
}

void* odDebugString_allocate(int32_t size, int32_t alignment) {
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
	uintptr_t aligned_allocation_uint = ((
		(allocation_uint + static_cast<uintptr_t>(alignment) - 1) / static_cast<uintptr_t>(alignment))
		* static_cast<uintptr_t>(alignment));
	// calculate offset and add that to pointer instead of casting (performance-no-int-to-ptr)
	uintptr_t aligned_offset_uint = aligned_allocation_uint - reinterpret_cast<uintptr_t>(allocation);
	char* aligned_allocation = allocation + aligned_offset_uint;

	return aligned_allocation;
}
const char* odDebugString_format_variadic(const char* format_c_str, va_list args) {
	if (!OD_CHECK(format_c_str != nullptr)) {
		return "\"<format_str_null>\"";
	}

	va_list compute_size_args = {};
	va_copy(compute_size_args, args);
	// passing a nullptr buffer to the sprintf-family of calls will only compute
	// the output size
	int required_count = vsnprintf(/*buffer*/ nullptr, /*bufsz*/ 0, format_c_str, compute_size_args);
	va_end(compute_size_args);

	if (!OD_CHECK(required_count >= 0)) {
		return "\"<debug_str_format_failed>\"";
	}

	// sprintf-style calls always write null-terminated, but count in return value
	// excludes null terminator
	int32_t required_capacity = static_cast<int32_t>(required_count) + 1;

	void* allocation = odDebugString_allocate(required_capacity, 1);
	if (!OD_CHECK(allocation != nullptr)) {
		return "\"<debug_str_allocation_failed>\"";
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
const char* odDebugString_format_array(const char* (*to_debug_str)(const void*),
									   const void* xs, int32_t count, int32_t stride) {
	if (count == 0) {
		return "";
	}

	if ((to_debug_str == nullptr) || (xs == nullptr) || (count <= 0)) {
		return "\"<debug_str_array_failed>\"";
	}

	const char** elem_strs = static_cast<const char**>(
		odDebugString_allocate(static_cast<int32_t>(sizeof(const char**)) * count, alignof(const char**)));
	if (elem_strs == nullptr) {
		return "\"<debug_str_array_failed>\"";
	}

	const char separator[] = ", ";
	const size_t separator_size = sizeof(separator) - 1;

	size_t combined_size = 0;
	combined_size += 1;  // null terminator
	combined_size += separator_size * static_cast<size_t>(count - 1);

	for (int32_t i = 0; i < count; i++) {
		const void* x = static_cast<const void*>(static_cast<const char*>(xs) + (i * stride));
		elem_strs[i] = to_debug_str(x);

		if (elem_strs[i] == nullptr) {
			return "\"<debug_str_array_failed>\"";
		}

		combined_size += strlen(elem_strs[i]);
	}

	char* combined = static_cast<char*>(odDebugString_allocate(static_cast<int32_t>(combined_size), 1));
	size_t combined_pos = 0;

	for (int32_t i = 0; i < count; i++) {
		size_t size = strlen(elem_strs[i]);
		strncpy(combined + combined_pos, elem_strs[i], size);
		combined_pos += size;

		if ((i + 1) < count) {
			strncpy(combined + combined_pos, separator, separator_size);
			combined_pos += separator_size;
		}
	}

	combined[combined_size - 1] = '\0';

	return combined;
}

odLogLevelScoped::odLogLevelScoped() : backup_log_level{odLogLevel_get_max()} {
}
odLogLevelScoped::odLogLevelScoped(int32_t log_level) : backup_log_level{odLogLevel_get_max()} {
	odLogLevel_set_max(log_level);
}
odLogLevelScoped::~odLogLevelScoped() {
	odLogLevel_set_max(backup_log_level);
}
