#pragma once

#include <od/test/test.h>

#if OD_BUILD_TESTS
#include <od/core/debug.hpp>

struct odTest {
	const char* suite;
	const char* name;
	void (*fn)();
	int32_t filters;

	OD_TEST_MODULE odTest(const char* in_suite, const char* in_name, void (*in_fn)(), int32_t in_filters);
};

// struct odTest {
// 	OD_TEST_MODULE odTest(const char* suite, const char* name, void (*fn)(), int32_t filters);
// };

#define OD_TEST_FILTERED(TEST_SUITE, TEST_NAME, TEST_FILTERS) \
	static void odTest_##TEST_SUITE##_##TEST_NAME(); \
	inline odTest odTest_get_##TEST_SUITE##_##TEST_NAME() { \
		return odTest{ \
			(#TEST_SUITE), \
			(#TEST_NAME), \
			&odTest_##TEST_SUITE##_##TEST_NAME, \
			(TEST_FILTERS), \
		}; \
	} \
	static void odTest_##TEST_SUITE##_##TEST_NAME()

#define OD_TEST(TEST_SUITE, TEST_NAME) OD_TEST_FILTERED(TEST_SUITE, TEST_NAME, OD_TEST_FILTER_NONE)

#endif
