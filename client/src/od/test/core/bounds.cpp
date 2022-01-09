#include <od/core/bounds.h>

#include <od/test/test.hpp>

OD_TEST(odTest_odBounds_collides) {
	odBounds bounds{8.0f, 8.0f, 72.0f, 72.0f};

	odBounds expected_collision_bounds[]{
		odBounds{8.0f, 8.0f, 9.0f, 9.0f},
		odBounds{71.0f, 71.0f, 72.0f, 72.0f},
		odBounds{7.0f, 7.0f, 9.0f, 9.0f},
		odBounds{7.0f, 8.0f, 9.0f, 9.0f},
		odBounds{8.0f, 7.0f, 9.0f, 9.0f},
		odBounds{7.0f, 7.0f, 71.0f, 71.0f},
		odBounds{7.0f, 7.0f, 73.0f, 73.0f},
	};

	odBounds expected_no_collision_bounds[]{
		odBounds{7.0f, 8.0f, 8.0f, 9.0f},
		odBounds{8.0f, 7.0f, 9.0f, 8.0f},
		odBounds{7.0f, 7.0f, 8.0f, 8.0f},
		odBounds{72.0f, 72.0f, 73.0f, 73.0f},
		odBounds{7.0f, 7.0f, 8.0f, 71.0f},
		odBounds{7.0f, 7.0f, 71.0f, 8.0f},
	};

	odBounds expected_not_collidable[]{
		odBounds{0.0f, 0.0f, 0.0f, 0.0f},
		odBounds{0.0f, 0.0f, 0.0f, 1.0f},
		odBounds{0.0f, 0.0f, 1.0f, 0.0f},
		odBounds{8.0f, 8.0f, 8.0f, 8.0f},
	};

	for (const odBounds& compare: expected_collision_bounds) {
		if (!odBounds_collides(&bounds, &compare)) {
			OD_ERROR("bounds expected to collide:\nbounds=%s,\ncompare=%s",
				odBounds_get_debug_string(&bounds), odBounds_get_debug_string(&compare));
		}
	}

	for (const odBounds& compare: expected_no_collision_bounds) {
		if (odBounds_collides(&bounds, &compare)) {
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
OD_TEST(odTest_odBounds_contains) {
	odBounds outer{-20.0f, 3.0f, -1.0f, 50.0f};

	odBounds expected_contained_bounds[]{
		outer,
		odBounds{outer.x1 + 1.0f, outer.y1, outer.x2, outer.y2},
		odBounds{outer.x1, outer.y1 + 1.0f, outer.x2, outer.y2},
		odBounds{outer.x1, outer.y1, outer.x2 - 1.0f, outer.y2},
		odBounds{outer.x1, outer.y1, outer.x2, outer.y2 - 1.0f},
		odBounds{outer.x1 + 1.0f, outer.y1 + 1.0f, outer.x2 - 1.0f, outer.y2 - 1.0f},
	};

	odBounds expected_not_contained_bounds[]{
		odBounds{outer.x1 - 1.0f, outer.y1, outer.x2, outer.y2},
		odBounds{outer.x1, outer.y1 - 1.0f, outer.x2, outer.y2},
		odBounds{outer.x1, outer.y1, outer.x2 + 1.0f, outer.y2},
		odBounds{outer.x1, outer.y1, outer.x2, outer.y2 + 1.0f},
		odBounds{outer.x1 - 1.0f, outer.y1 - 1.0f, outer.x2 + 1.0f, outer.y2 + 1.0f},
	};

	for (const odBounds& inner: expected_contained_bounds) {
		if (!odBounds_contains(&outer, &inner)) {
			OD_ERROR("outer bounds expected to contain inner:\nouter=%s,\ninner=%s",
				odBounds_get_debug_string(&outer), odBounds_get_debug_string(&inner));
		}
	}

	for (const odBounds& inner: expected_not_contained_bounds) {
		if (odBounds_contains(&outer, &inner)) {
			OD_ERROR("outer bounds expected to not contain inner:\nouter=%s,\ninner=%s",
				odBounds_get_debug_string(&outer), odBounds_get_debug_string(&inner));
		}
	}
}
OD_TEST(odTest_odBounds_equals) {
	odBounds bounds_comparisons[]{
		odBounds{0.0f, 0.0f, 0.0f, 0.0f},
		odBounds{0.0f, 0.0f, 0.0f, 1.0f},
		odBounds{0.0f, 0.0f, 1.0f, 0.0f},
		odBounds{8.0f, 8.0f, 8.0f, 8.0f},
	};
	for (const odBounds& bounds: bounds_comparisons) {
		OD_ASSERT(odBounds_equals(&bounds, &bounds));

		odBounds non_match{-1, -1, 0, 0};
		OD_ASSERT(!odBounds_equals(&bounds, &non_match));
	}
}
OD_TEST(odTest_odBounds_get_width) {
	odBounds bounds_width_8[]{
		odBounds{0.0f, 0.0f, 8.0f, 0.0f},
		odBounds{8.0f, 3.0f, 16.0f, 4.0f},
		odBounds{-8.0f, 11.0f, 0.0f, 11.0f},
	};
	for (const odBounds& bounds: bounds_width_8) {
		OD_ASSERT(odBounds_get_width(&bounds) == 8.0f);
	}

	odBounds bounds_width_0[]{
		odBounds{0.0f, 0.0f, 0.0f, 0.0f},
		odBounds{8.0f, 3.0f, 8.0f, 4.0f},
		odBounds{-8.0f, 11.0f, -8.0f, 11.0f},
	};
	for (const odBounds& bounds: bounds_width_0) {
		OD_ASSERT(odBounds_get_width(&bounds) == 0.0f);
	}
}
OD_TEST(odTest_odBounds_get_height) {
	odBounds bounds_height_8[]{
		odBounds{0.0f, 0.0f, 8.0f, 8.0f},
		odBounds{3.0f, 8.0f, 4.0f, 16.0f},
		odBounds{-8.0f, -8.0f, 3.0f, 0.0f},
	};
	for (const odBounds& bounds: bounds_height_8) {
		OD_ASSERT(odBounds_get_height(&bounds) == 8.0f);
	}

	odBounds bounds_height_0[]{
		odBounds{0.0f, 0.0f, 0.0f, 0.0f},
		odBounds{8.0f, 3.0f, 8.0f, 3.0f},
		odBounds{-8.0f, 11.0f, -8.0f, 11.0f},
	};
	for (const odBounds& bounds: bounds_height_0) {
		OD_ASSERT(odBounds_get_height(&bounds) == 0.0f);
	}
}

OD_TEST_SUITE(
	odTestSuite_odBounds,
	odTest_odBounds_collides,
	odTest_odBounds_contains,
	odTest_odBounds_equals,
	odTest_odBounds_get_width,
	odTest_odBounds_get_height,
)
