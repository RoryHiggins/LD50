#include <od/core/bounds.h>

#include <od/test/test.hpp>

OD_TEST(odTest_odBounds2f_collides) {
	odBounds2f bounds{8, 8, 72, 72};

	odBounds2f expected_collision_bounds[] = {
		{8, 8, 9, 9},
		{71, 71, 72, 72},
		{7, 7, 9, 9},
		{7, 8, 9, 9},
		{8, 7, 9, 9},
		{7, 7, 71, 71},
		{7, 7, 73, 73},
	};

	odBounds2f expected_no_collision_bounds[] = {
		{7, 8, 8, 9},
		{8, 7, 9, 8},
		{7, 7, 8, 8},
		{72, 72, 73, 73},
		{7, 7, 8, 71},
		{7, 7, 71, 8},
	};

	odBounds2f expected_not_collidable[] = {
		{0, 0, 0, 0},
		{0, 0, 0, 1},
		{0, 0, 1, 0},
		{8, 8, 8, 8},
	};

	for (const odBounds2f& compare: expected_collision_bounds) {
		if (!odBounds2f_collides(&bounds, &compare)) {
			OD_ERROR("bounds expected to collide:\nbounds=%s,\ncompare=%s",
				odBounds2f_get_debug_string(&bounds), odBounds2f_get_debug_string(&compare));
		}
	}

	for (const odBounds2f& compare: expected_no_collision_bounds) {
		if (!!odBounds2f_collides(&bounds, &compare)) {
			OD_ERROR("bounds expected not to collide:\nbounds=%s,\ncompare=%s",
				odBounds2f_get_debug_string(&bounds), odBounds2f_get_debug_string(&compare));
		}
	}

	for (const odBounds2f& non_collidable_bounds: expected_not_collidable) {
		if (odBounds2f_is_collidable(&non_collidable_bounds)) {
			OD_ERROR("bounds expected not to be collidable:\nbounds=%s", odBounds2f_get_debug_string(&non_collidable_bounds));
		}
	}
}
OD_TEST(odTest_odBounds2f_equals) {
	odBounds2f bounds_comparisons[] = {
		{0, 0, 0, 0},
		{0, 0, 0, 1},
		{0, 0, 1, 0},
		{8, 8, 8, 8},
	};
	for (const odBounds2f& bounds: bounds_comparisons) {
		OD_ASSERT(odBounds2f_equals(&bounds, &bounds));

		odBounds2f non_match{-1,-1,0,0};
		OD_ASSERT(!odBounds2f_equals(&bounds, &non_match));
	}
}
OD_TEST(odTest_odBounds2f_get_width) {
	odBounds2f bounds_width_8[] = {
		{0, 0, 8, 0},
		{8, 3, 16, 4},
		{-8, 11, 0, 11},
	};
	for (const odBounds2f& bounds: bounds_width_8) {
		OD_ASSERT(odBounds2f_get_width(&bounds) == 8);
	}

	odBounds2f bounds_width_0[] = {
		{0, 0, 0, 0},
		{8, 3, 8, 4},
		{-8, 11, -8, 11},
	};
	for (const odBounds2f& bounds: bounds_width_0) {
		OD_ASSERT(odBounds2f_get_width(&bounds) == 0);
	}
}
OD_TEST(odTest_odBounds2f_get_height) {
	odBounds2f bounds_height_8[] = {
		{0, 0, 8, 8},
		{3, 8, 4, 16},
		{-8, -8, 3, 0},
	};
	for (const odBounds2f& bounds: bounds_height_8) {
		OD_ASSERT(odBounds2f_get_height(&bounds) == 8);
	}

	odBounds2f bounds_height_0[] = {
		{0, 0, 0, 0},
		{8, 3, 8, 3},
		{-8, 11, -8, 11},
	};
	for (const odBounds2f& bounds: bounds_height_0) {
		OD_ASSERT(odBounds2f_get_height(&bounds) == 0);
	}
}
OD_TEST(odTest_odBounds2f_floor) {
	odBounds2f bounds{-2.1f, -2.5f, 3.4f, 3.99f};
	odBounds2f_floor(&bounds);

	odBounds2f expected_bounds{-3, -3, 3, 3};
	OD_ASSERT(odBounds2f_equals(&bounds, &expected_bounds));
}

OD_TEST_SUITE(
	odTestSuite_odBounds2f,
	odTest_odBounds2f_collides,
	odTest_odBounds2f_equals,
	odTest_odBounds2f_get_width,
	odTest_odBounds2f_get_height,
	odTest_odBounds2f_floor,
)
