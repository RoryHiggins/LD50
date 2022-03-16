#include <od/test/test.hpp>

#include <cstring>
#include <cstdlib>
#include <ctime>

#include <od/core/string.hpp>
#include <od/platform/timer.h>

bool odTest_run(int32_t filters, char const* opt_name_filter) {
	static const odTestSuite test_suites[] = {
		odTestSuite_odDebug(),
		odTestSuite_odAllocation(),
		odTestSuite_odArray(),
		odTestSuite_odBox(),
		odTestSuite_odBounds(),
		odTestSuite_odMatrix(),
		odTestSuite_odString(),

		odTestSuite_odAsciiFont(),
		odTestSuite_odFile(),
		odTestSuite_odImage(),
		odTestSuite_odWindow(),
		odTestSuite_odAudio(),
		odTestSuite_odTexture(),
		odTestSuite_odRenderTexture(),
		odTestSuite_odRenderer(),

		odTestSuite_odAtlas(),
		odTestSuite_odTextureAtlas(),
		odTestSuite_odEntityIndex(),
		odTestSuite_odLua(),
		odTestSuite_odLuaBindings(),
		odTestSuite_odLuaClient(),
	};

	OD_INFO("Running tests");
	if (filters & OD_TEST_FILTER_SLOW) {
		OD_INFO("Skipping slow tests");
	}

	if (opt_name_filter != nullptr) {
		OD_INFO("Skipping tests not matching name=\"%s\"", opt_name_filter);
	}

	odTimer timer;
	odTimer_start(&timer);

	int32_t total_test_count = 0;
	int32_t run_test_count = 0;
	for (odTestSuite suite: test_suites) {
		OD_DEBUG("Starting test suite \"%s\"", suite.name);

		for (int32_t i = 0; i < suite.tests_count; i++) {
			total_test_count++;

			odTest test = suite.tests[i];

			if ((opt_name_filter != nullptr) && (strstr(test.name, opt_name_filter) == nullptr)) {
				OD_DEBUG("Skipping test \"%s\"", test.name);
				continue;
			}

			if ((test.filters & filters) > 0) {
				OD_DEBUG("Skipping test \"%s\"", test.name);
				continue;
			}

			if (opt_name_filter) {
				OD_INFO("Starting test \"%s\"", test.name);
			} else {
				OD_DEBUG("Starting test \"%s\"", test.name);
			}

			int32_t logged_errors_before = odLog_get_logged_error_count();
			test.fn();
			run_test_count++;
			int32_t logged_errors_after = odLog_get_logged_error_count();
			int32_t new_errors = logged_errors_after - logged_errors_before;

			if (new_errors) {
				OD_ERROR("Failed test \"%s\"", test.name);
				return false;
			}

			OD_DEBUG("Successfully completed test \"%s\"", test.name);
		}

		OD_DEBUG("Successfully completed test suite \"%s\"", suite.name);
	}

	double time_elapsed = static_cast<double>(odTimer_get_elapsed_seconds(&timer));
	OD_INFO("Tests run successfully, %d run of %d in ~%g second(s)", run_test_count, total_test_count, time_elapsed);
	OD_MAYBE_UNUSED(time_elapsed);
	return true;
}
bool odTest_get_random_filename(struct odString* out_name) {
	const char prefix[] = "./odTest_file_";
	const int32_t prefix_size = sizeof(prefix) - 1;
	const int32_t random_size = 16;
	const int32_t size = prefix_size + random_size;
	OD_ASSERT(out_name->set_count(size));

	char* name_ptr = out_name->begin();
	OD_ASSERT(name_ptr != nullptr);

	memcpy(name_ptr, prefix, prefix_size);

	// mitigate collision risk from parallel test runs
	static bool random_seed_set = false;
	if (!random_seed_set) {
		unsigned seed =
			(static_cast<unsigned>(time(nullptr)) + static_cast<unsigned>(reinterpret_cast<uintptr_t>(name_ptr)));
		srand(seed);
		random_seed_set = true;
	}

	for (int32_t i = prefix_size; i < size; i++) {
		name_ptr[i] = static_cast<char>(static_cast<int>('a') + (rand() % 20));
	}

	return true;
}

odTest::odTest(const char* in_name, void (*in_fn)(), int32_t in_filters)
: name{in_name}, fn{in_fn}, filters{in_filters} {
}
