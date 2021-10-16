#pragma once

#include <od/test/test.h>

#if OD_BUILD_TESTS
#include <od/core/debug.hpp>

struct odTestInstantiator {
	OD_ENGINE_TEST_MODULE odTestInstantiator(const char* suite, const char* name, void (*fn)());
};

#define OD_TEST(TEST_SUITE, TEST_NAME) \
	static void odTest_##TEST_SUITE##_##TEST_NAME(void); \
	static const odTestInstantiator odTestInstantiator_##TEST_SUITE##_##TEST_NAME{\
		#TEST_SUITE, \
		#TEST_NAME, \
		&odTest_##TEST_SUITE##_##TEST_NAME}; \
	void odTest_##TEST_SUITE##_##TEST_NAME(void)

#endif
