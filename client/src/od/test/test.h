#pragma once

#include <od/test/module.h>
#include <od/core/debug.h>

#define OD_TEST_FILTER_NONE 0
#define OD_TEST_FILTER_SLOW 1

#if OD_BUILD_TESTS
OD_API_C OD_TEST_MODULE
void odTest_add(const char* suite, const char* name, void (*fn)(), int32_t filters);

OD_API_C OD_TEST_MODULE OD_NO_DISCARD
bool odTest_run(int32_t filters);
#else  // !OD_BUILD_TESTS
inline static void odTest_add(const char* /*suite*/, const char* /*name*/, void (*)(), int32_t /*filters*/) {
}

inline static bool odTest_run(int32_t /*filters*/) {
	return false;
}
#endif  // !OD_BUILD_TESTS
