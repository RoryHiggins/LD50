#include <od/core/debug.h>
#include <od/platform/window.hpp>
#include <od/test/test.hpp>

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

OD_NO_DISCARD static bool odClient_run() {
	OD_INFO("Running client");

	odWindow window;
	if (!OD_CHECK(odWindow_init(&window, odWindowSettings_get_defaults()))) {
		return false;
	}

	while (odWindow_get_valid(&window)) {
		if (!OD_CHECK(odWindow_step(&window))) {
			return false;
		}
	}

	OD_INFO("Client exited normally");
	return true;
}

int main(int argc, char** argv) {
	bool run_tests = false;
	bool run_client = true;
	int32_t test_filter = OD_TEST_FILTER_NONE;

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
		if (OD_BUILD_DEBUG) {
			if (strncmp(argv[i], "--trace", MAX_ARG_SIZE) == 0) {
				odLogLevel_set_max(OD_LOG_LEVEL_TRACE);
				continue;
			}
			if (strncmp(argv[i], "--debug", MAX_ARG_SIZE) == 0) {
				odLogLevel_set_max(OD_LOG_LEVEL_DEBUG);
				continue;
			}
		}
		if (strncmp(argv[i], "--no-log", MAX_ARG_SIZE) == 0) {
				odLogLevel_set_max(OD_LOG_LEVEL_NONE);
				continue;
			}
		if (strncmp(argv[i], "--test", MAX_ARG_SIZE) == 0) {
			run_tests = true;
			continue;
		}
		if (strncmp(argv[i], "--no-test", MAX_ARG_SIZE) == 0) {
			run_tests = false;
			continue;
		}
		if (strncmp(argv[i], "--slow-test", MAX_ARG_SIZE) == 0) {
			test_filter = test_filter & ~OD_TEST_FILTER_SLOW;
			continue;
		}
		if (strncmp(argv[i], "--no-slow-test", MAX_ARG_SIZE) == 0) {
			test_filter = test_filter | OD_TEST_FILTER_SLOW;
			continue;
		}
		if (strncmp(argv[i], "--client", MAX_ARG_SIZE) == 0) {
			run_client = true;
			continue;
		}
		if (strncmp(argv[i], "--no-client", MAX_ARG_SIZE) == 0) {
			run_client = false;
			continue;
		}
		if (i > 0) {
			OD_ERROR("Unknown argument \"%s\"", argv[i]);
			return 1;
		}
	}

	if (run_tests) {
#if (OD_BUILD_TESTS)
		if (!odTest_run(test_filter)) {
			OD_ERROR("tests failed");
			return 1;
		}
#else
		OD_ERROR("tests excluded from build");
		return 1;
#endif
	}

	if (run_client) {
		if (!odClient_run()) {
			OD_ERROR("client ended with error");
			return 1;
		}
	}

	return 0;
}
