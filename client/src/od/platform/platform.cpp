#include <od/platform/module.h>

#include <cfenv>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <od/core/api.h>
#include <od/core/debug.h>

#if OD_BUILD_LIBBACKTRACE
#include <backtrace.h>
#endif

#if OD_BUILD_DBGHELP
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#undef WIN32_LEAN_AND_MEAN
#undef VC_EXTRALEAN
#endif

#if defined(__GNUC__)
#include <cxxabi.h>
#endif

static bool odPlatform_backtrace_print();
extern "C" void odPlatform_float_reset_exceptions();
#if !OD_BUILD_EMSCRIPTEN
static int32_t odPlatform_float_get_enabled_exceptions();
#endif
static OD_NO_DISCARD bool odPlatform_float_check_exceptions(void);
static OD_NO_DISCARD bool odPlatform_float_init_exceptions();

#if OD_BUILD_DBGHELP
// libbacktrace does not play well with gcc + mingw, so we use the winapi here instead

bool odPlatform_backtrace_print() {
#if defined(_M_AMD64) && _M_AMD64
	auto check_windows_error = [](const char* message) -> bool {
		if (GetLastError()) {
			printf("%s, windows error code=%d\n", message, static_cast<int>(GetLastError()));
			SetLastError(0);

			return false;
		}

		return true;
	};

	SetLastError(0);

	HANDLE process = GetCurrentProcess();
	if (!check_windows_error("GetCurrentProcess")) {
		printf("failed to get current process\n");
		return false;
	}

	static bool sym_initialize_complete = false;
	if (!sym_initialize_complete) {
		SymSetOptions(SymGetOptions() | SYMOPT_LOAD_LINES | SYMOPT_LOAD_ANYTHING);
		BOOL sym_initialize_success = SymInitialize(
			process,
			/*UserSearchPath*/ nullptr,
			/*fInvadeProcess*/ TRUE
		);
		if (!check_windows_error("SymInitialize") || !sym_initialize_success) {
			printf("failed to initialize symbol handler for the current process\n");
				return false;
		}

		sym_initialize_complete = true;
	}

	const int32_t stacktrace_capacity = 64;
	int32_t stacktrace_count = 0;
	void* stacktrace_addrs[stacktrace_capacity]{};

	stacktrace_count = static_cast<int32_t>(RtlCaptureStackBackTrace(
  		/*FramesToSkip*/ 0,
  		/*FramesToCapture*/ stacktrace_capacity,
  		/*BackTrace*/ stacktrace_addrs,
  		/*BackTraceHash*/ nullptr
	));
	if (!check_windows_error("RtlCaptureStackBackTrace")) {
		printf("failed to capture stack trace\n");
		return false;
	}

	puts("--- backtrace ---");
	for (int32_t i = 0; i < stacktrace_count; i++) {
		const int32_t buffer_size = 256;
		char library[buffer_size] = "?";
		char filename[buffer_size] = "?";
		char function[buffer_size] = "?";
		int32_t line_num = -1;

		DWORD64 addr_dword = reinterpret_cast<DWORD64>(stacktrace_addrs[i]);

		IMAGEHLP_MODULE64 module_info{};
		module_info.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

		BOOL get_module_info_success = SymGetModuleInfo64(process, addr_dword, &module_info);
		if (!get_module_info_success) {
			// commented out as it makes stack traces unreadable
			// check_windows_error("SymGetModuleInfo64");
			// printf("failed to retrieve details on the symbol's module for this stack frame\n");
		} else {
			strncpy(library, module_info.LoadedImageName, buffer_size);
		}

		IMAGEHLP_LINE64 line_info{};
		line_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
		DWORD displacement_unused = 0;

		BOOL get_line_success = SymGetLineFromAddr64(process, addr_dword, &displacement_unused, &line_info);
		if (!get_line_success) {
			// commented out as it makes stack traces unreadable
			// check_windows_error("SymGetLineFromAddr64");
			// printf("failed to retrieve the source file line for this symbol\n");
		} else {
			strncpy(filename, line_info.FileName, buffer_size);
			line_num = static_cast<int32_t>(line_info.LineNumber);
		}

		// 8-byte aligned, at least big enough to fit a SYMBOL_INFO + a (MAX_SYM_NAME * sizeof(TCHAR)) string
		ULONG64 symbol_buffer[((sizeof(SYMBOL_INFO) + (MAX_SYM_NAME * sizeof(TCHAR))) + sizeof(ULONG64) - 1) / sizeof(ULONG64)];
		PSYMBOL_INFO symbol_info = reinterpret_cast<PSYMBOL_INFO>(symbol_buffer);
		symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol_info->MaxNameLen = MAX_SYM_NAME;
		DWORD64 displacement64_unused = 0;

		BOOL get_sym_success = SymFromAddr(process, addr_dword, &displacement64_unused, symbol_info);
		if (!get_sym_success) {
			// commented out as it makes stack traces unreadable
			// check_windows_error("SymFromAddr");
			// printf("failed to retrieve symbol infromation for the given addr\n");
		} else {
			strncpy(function, symbol_info->Name, buffer_size);
		}

		const size_t line_num_buf_size = 16;
		char line_num_buf[line_num_buf_size] = "?";
		if (line_num >= 0) {
			snprintf(line_num_buf, line_num_buf_size, "%d", line_num);
		}

#if defined(__GNUC__)
		if (strncmp(function, "?", buffer_size) != 0) {
			int status = 0;
			char* function_demangled = abi::__cxa_demangle(
				/*mangled_name*/ function,
				/*output_buffer*/ nullptr,
				/*length*/ nullptr,
				/*status*/ &status
			);

			// also try with an added _ prefix, which is required may get dropped
			if (status == -2) {
				char function_dash_prefix[buffer_size] = "_";
				strncpy(function_dash_prefix + 1, function, buffer_size - 1);

				function_demangled = abi::__cxa_demangle(
					/*mangled_name*/ function_dash_prefix,
					/*output_buffer*/ nullptr,
					/*length*/ nullptr,
					/*status*/ &status
				);
			}

			if ((status == 0) && (function_demangled != nullptr)) {
				strncpy(function, function_demangled, buffer_size);
			} else {
				// commented out as it makes stack traces unreadable
				// printf("failed, function=%s, status=%d\n", function, status);
			}

			if (function_demangled != nullptr) {
				free(function_demangled);
			}
		}
#endif

		char library_lowercase[buffer_size];
		for (int32_t j = 0; j < buffer_size; j++) {
			library_lowercase[j] = static_cast<char>(tolower(library[j]));
		}

		if (strstr(function, "odLog_") != nullptr
			|| strstr(function, "odDebug_") != nullptr
			|| strstr(library_lowercase, "\\windows\\") != nullptr
			|| strstr(library_lowercase, "\\system32\\") != nullptr
			|| strstr(library_lowercase, "\\syswow64\\") != nullptr) {
			continue;
		}

		printf("%s:%p\t%s %s:%s\n", library, stacktrace_addrs[i], function, filename, line_num_buf);
	}

	return true;
#else  // #if defined(_M_AMD64) && _M_AMD64
	return false;
#endif
}
#elif OD_BUILD_LIBBACKTRACE  // #if OD_BUILD_DBGHELP
static void odDebug_libbacktrace_error(void* /*data*/, const char* msg, int errnum)  {
	printf("libbacktrace error: errnum=%d, msg=%s\n", errnum, msg);
}
static backtrace_state* odDebug_create_backtrace_state() {
	const char* executable_filename = nullptr;
#if defined(_WIN32)
	static char executable_filename_buffer[MAX_PATH] = {};
	if (GetModuleFileNameA(nullptr, executable_filename_buffer, MAX_PATH) == 0) {
		printf("Failed to get windows executable filename, defaulting to autodetection\n");
	} else {
		executable_filename_buffer[MAX_PATH - 1] = '\0';
		executable_filename = executable_filename_buffer;
	}
#endif
	return backtrace_create_state(
		/*filename*/ executable_filename,
		/*threaded*/ 1,
		/*error_callback*/ &odDebug_libbacktrace_error,
		/*data*/ nullptr
	);
}
bool odPlatform_backtrace_print() {
	static backtrace_state* backtrace = odDebug_create_backtrace_state();

	puts("backtrace:\n");
	backtrace_print(backtrace, /*skip*/ 0, stdout);
	return true;
}
#else  // #elif OD_BUILD_LIBBACKTRACE  // #if OD_BUILD_DBGHELP
bool odPlatform_backtrace_print() {
	return false;
}
#endif
void odPlatform_float_reset_exceptions() {
	if (OD_BUILD_DEBUG) {
		OD_DISCARD(
			OD_CHECK(fesetenv(FE_DFL_ENV) == 0)
			&& OD_CHECK(odPlatform_float_check_exceptions()));
	}
}
#if !OD_BUILD_EMSCRIPTEN
static int32_t odPlatform_float_get_enabled_exceptions() {
	return FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW;
}
#endif
static OD_NO_DISCARD bool odPlatform_float_check_exceptions(void) {
#if !OD_BUILD_EMSCRIPTEN
	int float_exceptions = fetestexcept(odPlatform_float_get_enabled_exceptions());
	if (float_exceptions != 0) {
		const char* float_exception_str = "Unknown floating point exception or multiple exceptions";
		if (float_exceptions & FE_DIVBYZERO) {
			float_exception_str = "Division by 0";
		}
		if (float_exceptions & FE_INEXACT) {
			float_exception_str = "Inexact result";
		}
		if (float_exceptions & FE_INVALID) {
			float_exception_str = "Invalid operation";
		}
		if (float_exceptions & FE_OVERFLOW) {
			float_exception_str = "Overflowed";
		}
		if (float_exceptions & FE_UNDERFLOW) {
			float_exception_str = "Underflowed";
		}
		OD_ERROR(
			"Uncaught floating point exception(s) occurred: 0x%x: %s",
			static_cast<unsigned>(float_exceptions),
			float_exception_str);
		OD_MAYBE_UNUSED(float_exception_str);
		return false;
	}

	if (!OD_CHECK(feclearexcept(odPlatform_float_get_enabled_exceptions()) == 0)) {
		return false;
	}
#endif

	return true;
}
static OD_NO_DISCARD bool odPlatform_float_init_exceptions() {
#if !OD_BUILD_EMSCRIPTEN
	if (!OD_CHECK(feclearexcept(odPlatform_float_get_enabled_exceptions()) == 0)) {
		return false;
	}

	if (OD_BUILD_DEBUG) {
		fexcept_t float_env;
		if (!OD_CHECK(fegetexceptflag(&float_env, odPlatform_float_get_enabled_exceptions()) == 0)) {
			return false;
		}

		if (!OD_CHECK(fesetexceptflag(&float_env, odPlatform_float_get_enabled_exceptions()) == 0)) {
			return false;
		}

		if (!OD_CHECK(atexit(odPlatform_float_reset_exceptions) == 0)) {
			return false;
		}
	}
#endif

	return true;
}


struct odPlatform {
	odPlatform();
};

static odPlatform odPlatform_instance{};

odPlatform::odPlatform() {
	if (OD_BUILD_STACK_TRACES) {
		odDebug_set_backtrace_handler(&odPlatform_backtrace_print);
	}

	if (!OD_CHECK(odPlatform_float_init_exceptions())) {
		exit(1);
	}
}
