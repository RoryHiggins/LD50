#include <cstring>
#include <cstdlib>

#include <od/core/debug.h>
#include <od/engine/engine.hpp>
#include <od/test/test.hpp>

const char* __asan_default_options();

/* Parameters for AddressSanitizer; https://github.com/google/sanitizers/wiki/AddressSanitizerFlags */
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
	bool run_tests = false;
	bool run_client = true;
	int32_t test_filter = OD_TEST_FILTER_NONE;
	char const* opt_test_name_filter = nullptr;

	OD_TRACE("argc=%d", argc);
	for (int i = 0; i < argc; i++) {
		OD_TRACE("argv[%d]=%s", i, argv[i]);

		size_t arg_size = strlen(argv[i]);

		const char whitespace[] = " \t\n";
		if (strspn(argv[i], whitespace) == arg_size) {
			continue;
		}
		if (strncmp(argv[i], "--client", arg_size) == 0) {
			run_client = true;
			continue;
		}
		if (strncmp(argv[i], "--no-client", arg_size) == 0) {
			run_client = false;
			continue;
		}
		if (OD_BUILD_LOGS) {
			if (strncmp(argv[i], "--log", arg_size) == 0) {
				odLogLevel_set_max(OD_LOG_LEVEL_INFO);
				continue;
			}
			if (strncmp(argv[i], "--no-log", arg_size) == 0) {
				odLogLevel_set_max(OD_LOG_LEVEL_NONE);
				continue;
			}
		}
		if (OD_BUILD_LOGS && OD_BUILD_DEBUG) {
			if (strncmp(argv[i], "--debug", arg_size) == 0) {
				odLogLevel_set_max(OD_LOG_LEVEL_DEBUG);
				continue;
			}
			if (strncmp(argv[i], "--trace", arg_size) == 0) {
				odLogLevel_set_max(OD_LOG_LEVEL_TRACE);
				continue;
			}
		}
		if (OD_BUILD_TESTS) {
			if (strncmp(argv[i], "--test", arg_size) == 0) {
				run_tests = true;
				continue;
			}
			if (strncmp(argv[i], "--no-test", arg_size) == 0) {
				run_tests = false;
				continue;
			}
			if (strncmp(argv[i], "--slow-test", arg_size) == 0) {
				test_filter = test_filter & ~OD_TEST_FILTER_SLOW;
				continue;
			}
			if (strncmp(argv[i], "--no-slow-test", arg_size) == 0) {
				test_filter = test_filter | OD_TEST_FILTER_SLOW;
				continue;
			}
			if (strncmp(argv[i], "--test-name", arg_size) == 0) {
				if (((i + 1) >= argc) || (strcmp(argv[i + 1], "") == 0)) {
					OD_ERROR("Missing value for --test-name");
					return 1;
				}

				i++;
				opt_test_name_filter = argv[i];
				continue;
			}
		}
		if (i > 0) {
			OD_ERROR("Unknown argument \"%s\"", argv[i]);
			return 1;
		}
	}

	if (run_tests) {
		if (!OD_CHECK(OD_BUILD_TESTS) || !odTest_run(test_filter, opt_test_name_filter)) {
			return 1;
		}
	}

	if (run_client) {
		odEngine engine;
		odEngineSettings engine_settings{*odEngineSettings_get_defaults()};
		if (!odEngine_run(&engine, &engine_settings)) {
			return 1;
		}
	}

	return 0;
}
