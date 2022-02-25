#pragma once

#include <od/test/module.h>
#include <od/core/debug.h>

#define OD_TEST_FILTER_NONE 0
#define OD_TEST_FILTER_SLOW 1

#if OD_BUILD_TESTS
OD_API_C OD_TEST_MODULE OD_NO_DISCARD bool
odTest_run(int32_t filters, char const* opt_name_filter);
OD_API_C OD_TEST_MODULE OD_NO_DISCARD bool
odTest_get_random_filename(struct odString* out_name);
#endif
