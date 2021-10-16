#include <od/core/debug.h>
#include <od/platform/window.hpp>

#include <cstring>

/* Parameters for AddressSanitizer; https://github.com/google/sanitizers/wiki/AddressSanitizerFlags */
const char* __asan_default_options();
const char* __asan_default_options() {
	return ("verbosity=1"
			":halt_on_error=0"
			":strict_string_checks=1"
			":detect_stack_use_after_return=1"
			":check_initialization_order=1"
			":strict_init_order=1"
			":detect_invalid_pointer_pairs=10"
			":detect_leaks=1");
}

int main(int argc, char** argv) {
	const uint32_t MAX_ARG_SIZE = 64;
	if (argv == nullptr) {
		OD_ERROR("Unexpected empty argument array");
		return 1;
	}
	for (int i = 0; i < argc; i++) {
		if (argv[i] == nullptr) {
			OD_ERROR("Unexpected empty argument");
			return 1;
		}
		if (strncmp(argv[i], "--trace", MAX_ARG_SIZE) == 0) {
			odLogLevel_set_max(OD_LOG_LEVEL_TRACE);
			continue;
		}
		if (strncmp(argv[i], "--debug", MAX_ARG_SIZE) == 0) {
			odLogLevel_set_max(OD_LOG_LEVEL_DEBUG);
			continue;
		}
		if (i > 0) {
			OD_ERROR("Unknown argument \"%s\"", argv[i]);
			return 1;
		}
	}

	odWindow window;
	if (!odWindow_init(&window, odWindowSettings_get_defaults())) {
		return 1;
	}

	while (odWindow_get_open(&window)) {
		odWindow_step(&window);
	}

	return 0;
}
