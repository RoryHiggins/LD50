#pragma once

#include <od/test/test.h>

#if OD_BUILD_TESTS
#include <od/core/debug.hpp>

struct odTest {
	const char* name;
	void (*fn)();
	int32_t filters;

	OD_TEST_MODULE odTest(const char* in_name, void (*in_fn)(), int32_t in_filters);
};

struct odTestSuite {
	const char* name;
	const odTest* tests;
	int32_t tests_count;
};

#define OD_TEST_FILTERED(TEST_NAME, TEST_FILTERS) \
	static void TEST_NAME##_run(); \
	static odTest TEST_NAME = odTest{#TEST_NAME, &TEST_NAME##_run, (TEST_FILTERS)}; \
	static void TEST_NAME##_run()

#define OD_TEST(TEST_NAME) OD_TEST_FILTERED(TEST_NAME, OD_TEST_FILTER_NONE)

#define OD_TEST_SUITE_DECLARE(SUITE_NAME) extern OD_TEST_MODULE odTestSuite SUITE_NAME();

#define OD_TEST_SUITE(SUITE_NAME, ...) \
	odTestSuite SUITE_NAME() { \
		static odTest tests[] = { __VA_ARGS__ }; \
		return odTestSuite{#SUITE_NAME, tests, (sizeof(tests) / sizeof(odTest))}; \
	}

OD_TEST_SUITE_DECLARE(odTestSuite_odDebug)
OD_TEST_SUITE_DECLARE(odTestSuite_odAllocation)
OD_TEST_SUITE_DECLARE(odTestSuite_odArray)
OD_TEST_SUITE_DECLARE(odTestSuite_odBounds)
OD_TEST_SUITE_DECLARE(odTestSuite_odMatrix)
OD_TEST_SUITE_DECLARE(odTestSuite_odString)

OD_TEST_SUITE_DECLARE(odTestSuite_odFile)
OD_TEST_SUITE_DECLARE(odTestSuite_odImage)
OD_TEST_SUITE_DECLARE(odTestSuite_odWindow)
OD_TEST_SUITE_DECLARE(odTestSuite_odTexture)
OD_TEST_SUITE_DECLARE(odTestSuite_odRenderer)

OD_TEST_SUITE_DECLARE(odTestSuite_odAtlas)
OD_TEST_SUITE_DECLARE(odTestSuite_odEntityIndex)
OD_TEST_SUITE_DECLARE(odTestSuite_odLua)
OD_TEST_SUITE_DECLARE(odTestSuite_odLuaClient)

#endif
