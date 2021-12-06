#include <od/test/test.hpp>

#include <cstring>

#include <od/test/core/allocation.cpp>
#include <od/test/core/fast_array.cpp>
#include <od/test/core/array.cpp>
#include <od/test/core/bounds.cpp>
#include <od/test/core/debug.cpp>
#include <od/test/core/string.cpp>
#include <od/test/core/type.cpp>
#include <od/test/platform/file.cpp>
#include <od/test/platform/image.cpp>
#include <od/test/platform/rendering.cpp>
#include <od/test/platform/window.cpp>
#include <od/test/engine/entity_index.cpp>

static const odTest odTest_tests[] = {
	odTest_get_odFastArray_swap(),
	odTest_get_odFastArray_set_capacity(),
	odTest_get_odFastArray_set_capacity_zero(),
	odTest_get_odFastArray_set_count(),
	odTest_get_odFastArray_swap_pop(),
	odTest_get_odFastArrayT_foreach(),
	odTest_get_odFastArrayT_extend(),

	odTest_get_odAllocation_swap(),
	odTest_get_odAllocation_swap_unallocated(),
	odTest_get_odAllocation_init_destroy(),
	odTest_get_odAllocation_allocate_zero(),
	odTest_get_odAllocation_get(),
	odTest_get_odAllocation_get_unallocated_fails(),

	odTest_get_odArray_swap(),
	odTest_get_odArray_init_destroy(),
	odTest_get_odArray_set_capacity(),
	odTest_get_odArray_set_capacity_zero(),
	odTest_get_odArray_ensure_capacity(),
	odTest_get_odArray_set_count(),
	odTest_get_odArray_set_count_expand(),
	odTest_get_odArray_set_count_truncate(),
	odTest_get_odArray_pop(),
	odTest_get_odArray_swap_pop(),
	odTest_get_odArray_get_out_of_bounds_fails(),
	odTest_get_odArrayT_get(),
	odTest_get_odArrayT_foreach(),
	odTest_get_odArrayT_push(),
	odTest_get_odArrayT_debug_get_out_of_bounds_fails(),
	odTest_get_odArrayT_push_pop_container(),

	odTest_get_odBounds_collides(),

	odTest_get_odLog_get_level_name(),

	odTest_get_odString_swap(),
	odTest_get_odString_init_destroy(),
	odTest_get_odString_set_capacity(),
	odTest_get_odString_set_capacity_zero(),
	odTest_get_odString_ensure_capacity(),
	odTest_get_odString_set_count(),
	odTest_get_odString_set_count_expand(),
	odTest_get_odString_set_count_truncate(),
	odTest_get_odString_debug_get_out_of_bounds_fails(),
	odTest_get_odString_compare(),
	odTest_get_odString_copy(),
	odTest_get_odString_push_formatted(),

	odTest_get_odType_index(),

	odTest_get_odFile_open(),
	odTest_get_odFile_write_read_delete(),
	odTest_get_odFilePath_write_read_delete(),

	odTest_get_odImage_allocate_release(),
	odTest_get_odImage_read_png(),
	odTest_get_odImage_read_invalid_png_fails(),

	odTest_get_odTexture_init_destroy(),
	odTest_get_odTexture_init_large(),
	odTest_get_odTexture_destroy_after_window_destroy_fails(),
	odTest_get_odTexture_get_size(),
	odTest_get_odTexture_init_without_context_fails(),
	odTest_get_odTexture_destroy_invalid(),
	odTest_get_odRenderTexture_init_destroy(),
	odTest_get_odRenderTexture_init_large(),
	odTest_get_odRenderTexture_destroy_after_window_destroy_fails(),
	odTest_get_odRenderTexture_init_without_context_fails(),
	odTest_get_odRenderTexture_destroy_invalid(),
	odTest_get_odRenderer_init_destroy(),
	odTest_get_odRenderer_destroy_after_window_destroy_fails(),
	odTest_get_odRenderer_flush(),
	odTest_get_odRenderer_clear(),
	odTest_get_odRenderer_draw_vertices(),
	odTest_get_odRenderer_draw_texture(),
	odTest_get_odRenderer_init_multiple_renderers(),
	odTest_get_odRenderer_init_without_context_fails(),
	odTest_get_odRenderer_destroy_invalid(),

	odTest_get_odWindow_init_destroy(),
	odTest_get_odWindow_step(),
	odTest_get_odWindow_set_visible(),
	odTest_get_odWindow_set_visible_not_open_fails(),
	odTest_get_odWindow_set_size(),
	odTest_get_odWindow_get_open(),
	odTest_get_odWindow_init_multiple_windows(),
	odTest_get_odWindow_destroy_invalid(),

	odTest_get_odEntityIndex_init_destroy(),
	odTest_get_odEntityIndex_set_get(),
	odTest_get_odEntityIndex_search(),
	odTest_get_odEntityIndex_search_performance_realistic_case(),
};

static const int32_t odTest_test_count = sizeof(odTest_tests) / sizeof(odTest);

bool odTest_run(int32_t filters, char const* opt_name_filter) {
	OD_INFO("Running tests");
	if (filters & OD_TEST_FILTER_SLOW) {
		OD_INFO("Skipping slow tests");
	}

	if (opt_name_filter != nullptr) {
		OD_INFO("Skipping tests not matching name=\"%s\"", opt_name_filter);
	}
	
	int32_t run_test_count = 0;
	for (int32_t i = 0; i < odTest_test_count; i++) {
		if ((opt_name_filter != nullptr) && (strstr(odTest_tests[i].name, opt_name_filter) == nullptr)) {
			OD_DEBUG("Skipping test %s.%s", odTest_tests[i].suite, odTest_tests[i].name);
			continue;
		}

		if ((odTest_tests[i].filters & filters) > 0) {
			OD_DEBUG("Skipping test %s.%s", odTest_tests[i].suite, odTest_tests[i].name);
			continue;
		}

		if (opt_name_filter) {
			OD_INFO("Starting test %s.%s", odTest_tests[i].suite, odTest_tests[i].name);
		} else {
			OD_DEBUG("Starting test %s.%s", odTest_tests[i].suite, odTest_tests[i].name);
		}

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

odTest::odTest(const char* in_suite, const char* in_name, void (*in_fn)(), int32_t in_filters)
: suite{in_suite}, name{in_name}, fn{in_fn}, filters{in_filters} {
}
