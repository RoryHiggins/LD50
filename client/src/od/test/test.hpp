#pragma once

#include <od/test/test.h>

#if OD_BUILD_TESTS
#include <od/core/debug.hpp>

struct odTestInstantiator {
	OD_TEST_MODULE odTestInstantiator(const char* suite, const char* name, void (*fn)(), int32_t filters);
};

#define OD_TEST_FILTERED(TEST_SUITE, TEST_NAME, TEST_FILTERS) \
	static void odTest_##TEST_SUITE##_##TEST_NAME(void); \
	static const odTestInstantiator odTestInstantiator_##TEST_SUITE##_##TEST_NAME{\
		(#TEST_SUITE), \
		(#TEST_NAME), \
		&odTest_##TEST_SUITE##_##TEST_NAME, \
		(TEST_FILTERS), \
	}; \
	void odTest_##TEST_SUITE##_##TEST_NAME(void)

#define OD_TEST(TEST_SUITE, TEST_NAME) OD_TEST_FILTERED(TEST_SUITE, TEST_NAME, OD_TEST_FILTER_NONE)

#endif
