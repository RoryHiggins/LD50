#pragma once

#include <od/test/module.h>

#if OD_BUILD_TESTS
#include <od/core/debug.h>

OD_API_C OD_ENGINE_TEST_MODULE void odTest_add(const char* suite, const char* name, void (*fn)());
OD_API_C OD_ENGINE_TEST_MODULE void odTest_run_all(void);
#endif
