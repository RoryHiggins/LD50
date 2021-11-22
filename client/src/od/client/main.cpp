#include <od/core/debug.h>
#include <od/platform/window.hpp>
#include <od/test/test.hpp>

#include <cstring>

#if OD_BUILD_EMSCRIPTEN
#include <emscripten.h>
#endif   // OD_BUILD_EMSCRIPTEN

struct odClient {
	odWindow window;
	bool is_initialized;

	odClient();
};

odClient::odClient()
	: window{}, is_initialized{false} {
}

OD_NO_DISCARD static bool odClient_init(odClient* client) {
	odWindowSettings window_settings{odWindowSettings_get_defaults()};
	return odWindow_init(&client->window, window_settings);
}
OD_NO_DISCARD static bool odClient_step(odClient* client) {
	if (!client->is_initialized) {
		if (!OD_CHECK(odClient_init(client))) {
			return false;
		}
		client->is_initialized = true;
	}

	if (!OD_CHECK(odWindow_get_valid(&client->window))) {
		return false;
	}

	if (!OD_CHECK(odWindow_step(&client->window)) || !odWindow_get_valid(&client->window)) {
		return false;
	}

	return true;
}

#if OD_BUILD_EMSCRIPTEN
static void odClient_step_emscripten(void* client_raw) {
	odClient* client = reinterpret_cast<odClient*>(client_raw);
	if (!odClient_step(client)) {
		emscripten_cancel_main_loop();
	}
}
OD_NO_DISCARD static bool odClient_run(odClient* client) {
	emscripten_set_main_loop_arg(&odClient_step_emscripten, reinterpret_cast<void*>(client), 0, true);

	return true;
}
#else
OD_NO_DISCARD static bool odClient_run(odClient* client) {
	OD_INFO("Running client");

	int32_t logged_errors_before = odLog_get_logged_error_count();

	while (odClient_step(client)) {
	}

	if (odLog_get_logged_error_count() > logged_errors_before) {
		OD_INFO("Client ended with errors");
		return false;
	}

	OD_INFO("Client ended gracefully");
	return true;
}
#endif


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
	bool run_tests = false;
	bool run_client = true;
	int32_t test_filter = OD_TEST_FILTER_NONE;

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
		if (OD_BUILD_LOG) {
			if (strncmp(argv[i], "--log", arg_size) == 0) {
				odLogLevel_set_max(OD_LOG_LEVEL_INFO);
				continue;
			}
			if (strncmp(argv[i], "--no-log", arg_size) == 0) {
				odLogLevel_set_max(OD_LOG_LEVEL_NONE);
				continue;
			}
		}
		if (OD_BUILD_LOG && OD_BUILD_DEBUG) {
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
		}
		if (i > 0) {
			OD_ERROR("Unknown argument \"%s\"", argv[i]);
			return 1;
		}
	}

	if (run_tests) {
		if (!OD_CHECK(OD_BUILD_TESTS) || !odTest_run(test_filter)) {
			return 1;
		}
	}

	if (run_client) {
		odClient client;
		if (!odClient_run(&client)) {
			return 1;
		}
	}

	return 0;
}
