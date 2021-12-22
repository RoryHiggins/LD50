#include <od/core/matrix.h>

#include <od/core/vector.h>
#include <od/test/test.hpp>

OD_TEST(odTest_odMatrix_init) {
	odMatrix4 matrix{};
	odMatrix4_init(&matrix, 1, 1, 1, 0, 0, 0);

	OD_ASSERT(odMatrix4_equals(&matrix, odMatrix4_get_identity()));

	for (float translation = 0; translation < 4; translation++) {
		for (float scale = 1; scale < 4; scale++) {
			odMatrix4_init(&matrix, scale, scale, scale, translation, translation, translation);

			odVector4 vector{1, 1, 1, 1};
			odVector4 expected_vector{scale + translation, scale + translation, scale + translation, 1};
			odMatrix4_multiply_vector(&matrix, &vector);
			OD_ASSERT(odVector4_equals(&vector, &expected_vector));
		}
	}
}
OD_TEST(odTest_odMatrix_init_view_2d) {
	const int32_t view_width = 256;
	const int32_t view_height = 256;

	odMatrix4 view_matrix{};
	odMatrix4_init_view_2d(&view_matrix, view_width, view_height);

	odVector4 view_origin{};
	odMatrix4_multiply_vector(&view_matrix, &view_origin);

	// OD_INFO("%s", odMatrix4_get_debug_string(&view_matrix));
	const odVector4 expected_view_origin{};
	OD_ASSERT(odVector4_equals(&view_origin, &expected_view_origin));

	odVector4 view_end{view_width, view_height, 0, 0};
	const odVector4 expected_view_end{1, -1};
	odMatrix4_multiply_vector(&view_matrix, &view_end);
	OD_INFO("%s", odVector4_get_debug_string(&view_end));
	// OD_ASSERT(odVector4_equals(&view_end, &expected_view_end));
}

OD_TEST_SUITE(
	odTestSuite_odMatrix,
	odTest_odMatrix_init,
	odTest_odMatrix_init_view_2d,
)
