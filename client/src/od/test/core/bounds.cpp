#include <od/core/bounds.h>

#include <od/test/test.hpp>

OD_TEST(odTest_odBounds_collides) {
	odBounds bounds{8, 8, 72, 72};

	odBounds expected_collision_bounds[] = {
		{8, 8, 9, 9},
		{71, 71, 72, 72},
		{7, 7, 9, 9},
		{7, 8, 9, 9},
		{8, 7, 9, 9},
		{7, 7, 71, 71},
		{7, 7, 73, 73},
	};

	odBounds expected_no_collision_bounds[] = {
		{7, 8, 8, 9},
		{8, 7, 9, 8},
		{7, 7, 8, 8},
		{72, 72, 73, 73},
		{7, 7, 8, 71},
		{7, 7, 71, 8},
	};

	odBounds expected_not_collidable[] = {
		{0, 0, 0, 0},
		{0, 0, 0, 1},
		{0, 0, 1, 0},
		{8, 8, 8, 8},
	};

	for (const odBounds& compare: expected_collision_bounds) {
		if (!odBounds_collides(&bounds, &compare)) {
			OD_ERROR("bounds expected to collide:\nbounds=%s,\ncompare=%s",
				odBounds_get_debug_string(&bounds), odBounds_get_debug_string(&compare));
		}
	}

	for (const odBounds& compare: expected_no_collision_bounds) {
		if (!!odBounds_collides(&bounds, &compare)) {
			OD_ERROR("bounds expected not to collide:\nbounds=%s,\ncompare=%s",
				odBounds_get_debug_string(&bounds), odBounds_get_debug_string(&compare));
		}
	}

	for (const odBounds& non_collidable_bounds: expected_not_collidable) {
		if (odBounds_is_collidable(&non_collidable_bounds)) {
			OD_ERROR("bounds expected not to be collidable:\nbounds=%s", odBounds_get_debug_string(&non_collidable_bounds));
		}
	}
}
OD_TEST(odTest_odBounds_equals) {
	odBounds bounds_comparisons[] = {
		{0, 0, 0, 0},
		{0, 0, 0, 1},
		{0, 0, 1, 0},
		{8, 8, 8, 8},
	};
	for (const odBounds& bounds: bounds_comparisons) {
		OD_ASSERT(odBounds_equals(&bounds, &bounds));

		odBounds non_match{-1,-1,0,0};
		OD_ASSERT(!odBounds_equals(&bounds, &non_match));
	}
}
OD_TEST(odTest_odBounds_get_width) {
	odBounds bounds_width_8[] = {
		{0, 0, 8, 0},
		{8, 3, 16, 4},
		{-8, 11, 0, 11},
	};
	for (const odBounds& bounds: bounds_width_8) {
		OD_ASSERT(odBounds_get_width(&bounds) == 8);
	}

	odBounds bounds_width_0[] = {
		{0, 0, 0, 0},
		{8, 3, 8, 4},
		{-8, 11, -8, 11},
	};
	for (const odBounds& bounds: bounds_width_0) {
		OD_ASSERT(odBounds_get_width(&bounds) == 0);
	}
}
OD_TEST(odTest_odBounds_get_height) {
	odBounds bounds_height_8[] = {
		{0, 0, 8, 8},
		{3, 8, 4, 16},
		{-8, -8, 3, 0},
	};
	for (const odBounds& bounds: bounds_height_8) {
		OD_ASSERT(odBounds_get_height(&bounds) == 8);
	}

	odBounds bounds_height_0[] = {
		{0, 0, 0, 0},
		{8, 3, 8, 3},
		{-8, 11, -8, 11},
	};
	for (const odBounds& bounds: bounds_height_0) {
		OD_ASSERT(odBounds_get_height(&bounds) == 0);
	}
}

OD_TEST_SUITE(
	odTestSuite_odBounds,
	odTest_odBounds_collides,
	odTest_odBounds_equals,
	odTest_odBounds_get_width,
	odTest_odBounds_get_height,
)
