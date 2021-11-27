#include <od/core/bounds.h>

#include <od/test/test.hpp>

OD_TEST(odBounds, collides) {
	odBounds bounds{8, 8, 64, 64};

	odBounds expected_collision_bounds[] = {
		{8, 8, 1, 1},
		{71, 71, 1, 1},
		{7, 7, 2, 2},
		{7, 8, 2, 1},
		{8, 7, 1, 2},
		{7, 7, 64, 64},
		{7, 7, 66, 66},
	};

	odBounds expected_no_collision_bounds[] = {
		{8, 8, 0, 0},
		{9, 9, 0, 0},
		{7, 8, 1, 1},
		{8, 7, 1, 1},
		{7, 7, 1, 1},
		{72, 72, 1, 1},
		{7, 7, 1, 64},
		{7, 7, 64, 1},
		{8, 8, 0, 0},
	};

	for (const odBounds& compare: expected_collision_bounds) {
		if (!odBounds_collides(&bounds, &compare)) {
			OD_ERROR("bounds expected to collide but do not collide:\nbounds=%s,\ncompare=%s",
				odBounds_get_debug_string(&bounds), odBounds_get_debug_string(&compare));
		}
	}
	for (const odBounds& compare: expected_no_collision_bounds) {
		if (!!odBounds_collides(&bounds, &compare)) {
			OD_ERROR("bounds expected not to collide but do collide:\nbounds=%s,\ncompare=%s",
				odBounds_get_debug_string(&bounds), odBounds_get_debug_string(&compare));
		}
	}
}
