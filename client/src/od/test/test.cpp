#include <od/test/test.hpp>

#define OD_TEST_MAX_COUNT 1024

struct odTest {
	const char* suite;
	const char* name;
	void (*fn)();
};
static int32_t odTest_test_count = 0;
static odTest odTest_tests[OD_TEST_MAX_COUNT];

void odTest_add(const char* suite, const char* name, void (*fn)()) {
	if (suite == nullptr) {
		OD_ERROR("suite == nullptr");
	}
	if (name == nullptr) {
		OD_ERROR("name == nullptr");
	}
	if (fn == nullptr) {
		OD_ERROR("fn == nullptr");
	}

	odTest_tests[odTest_test_count] = {suite, name, fn};
	odTest_test_count++;
}

void odTest_run_all() {
	OD_INFO("Running all tests, odTest_test_count=%d", odTest_test_count);
	
	for (int32_t i = 0; i < odTest_test_count; i++) {
		OD_DEBUG("Running test %s.%s", odTest_tests[i].suite, odTest_tests[i].name);
		odTest_tests[i].fn();
	}

	OD_INFO("Tests completed successfully, odTest_test_count=%d", odTest_test_count);
}

odTestInstantiator::odTestInstantiator(const char* suite, const char* name, void (*fn)()) {
	odTest_add(suite, name, fn);
}
