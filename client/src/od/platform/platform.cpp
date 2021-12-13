#include <od/platform/module.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <od/core/debug.h>

#if OD_BUILD_DEBUG && defined(OD_CORE_HAS_LIBBACKTRACE) && OD_CORE_HAS_LIBBACKTRACE
#include <backtrace.h>
#endif

#if OD_BUILD_DEBUG && defined(_WIN32) && defined(OD_CORE_HAS_DBGHELP) && OD_CORE_HAS_DBGHELP

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#undef WIN32_LEAN_AND_MEAN
#undef VC_EXTRALEAN
#if defined(__GNUC__)
#include <cxxabi.h>
#endif
#endif

bool odPlatform_backtrace_print();

#if OD_BUILD_DEBUG && defined(_WIN32) && defined(OD_CORE_HAS_DBGHELP) && OD_CORE_HAS_DBGHELP
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

	puts("backtrace:");
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

		printf("%s:%p\n\t%s %s:%s\n",
			   library, stacktrace_addrs[i], function, filename, line_num_buf);
	}

	return true;
#else  // #if defined(_M_AMD64) && _M_AMD64
#warning "Windows architecture not supported for callstacks"
	return false;
#endif
}
#elif OD_BUILD_DEBUG && defined(OD_CORE_HAS_LIBBACKTRACE) && OD_CORE_HAS_LIBBACKTRACE
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
#else
bool odPlatform_backtrace_print() {
	return false;
}
#endif


struct odPlatform {
	odPlatform();
};

static odPlatform odPlatform_instance{};

odPlatform::odPlatform() {
	odDebug_set_backtrace_handler(&odPlatform_backtrace_print);
}
