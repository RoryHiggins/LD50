#include <od/test/test.hpp>

#define OD_TEST_MAX_COUNT 1024

struct odTest {
	const char* suite;
	const char* name;
	void (*fn)();
	int32_t filters;
};
static int32_t odTest_test_count = 0;
static odTest odTest_tests[OD_TEST_MAX_COUNT];

void odTest_add(const char* suite, const char* name, void (*fn)(), int32_t filters) {
	if (suite == nullptr) {
		OD_ERROR("suite == nullptr");
	}
	if (name == nullptr) {
		OD_ERROR("name == nullptr");
	}
	if (fn == nullptr) {
		OD_ERROR("fn == nullptr");
	}

	odTest_tests[odTest_test_count] = {suite, name, fn, filters};
	odTest_test_count++;
}

bool odTest_run(int32_t filters) {
	OD_INFO("Running tests");
	if (filters & OD_TEST_FILTER_SLOW) {
		OD_INFO("Skipping slow tests");
	}
	
	int32_t run_test_count = 0;
	for (int32_t i = 0; i < odTest_test_count; i++) {
		if ((odTest_tests[i].filters & filters) > 0) {
			OD_DEBUG("Skipping test %s.%s", odTest_tests[i].suite, odTest_tests[i].name);
			continue;
		}

		OD_DEBUG("Starting test %s.%s", odTest_tests[i].suite, odTest_tests[i].name);

		int32_t logged_errors_before = odLog_get_logged_error_count();
		odTest_tests[i].fn();
		run_test_count++;
		int32_t logged_errors_after = odLog_get_logged_error_count();
		int32_t new_errors = logged_errors_after - logged_errors_before;

		if (new_errors) {
			OD_ERROR("Failed test %s.%s", odTest_tests[i].suite, odTest_tests[i].name);
			return false;
		}

		OD_DEBUG("Completed test successfully %s.%s", odTest_tests[i].suite, odTest_tests[i].name);
	}

	OD_INFO("Tests run successfully, %d run of %d", run_test_count, odTest_test_count);
	return true;
}

odTestInstantiator::odTestInstantiator(const char* suite, const char* name, void (*fn)(), int32_t filters) {
	odTest_add(suite, name, fn, filters);
}
