#include <od/core/matrix.h>
#include <od/core/vector.h>
#include <od/test/test.hpp>

// TODO remove
#include <cfloat>
#include <od/core/math.h>

OD_TEST(odTest_odMatrix_init) {
	odMatrix matrix{};
	odMatrix_init(&matrix, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f);

	OD_ASSERT(odMatrix_equals(&matrix, odMatrix_get_identity()));

	float translation = 4.0f;
	float scale = 3.0f;
	odMatrix_init(&matrix, scale, scale, scale, translation, translation, translation);

	odVector vector{1.0f, 1.0f, 1.0f, 1.0f};
	odVector expected_vector{scale + translation, scale + translation, scale + translation, 1.0f};
	odMatrix_multiply_vector_3d(&matrix, &vector);
	OD_ASSERT(odVector_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix_init_ortho_2d) {
	const int32_t projection_width = 256;
	const int32_t projection_height = 256;

	odMatrix matrix{};
	odMatrix_init_ortho_2d(&matrix, projection_width, projection_height);

	odVector vector{0.0f, 0.0f, 0.0f, 1.0f};
	odVector expected_vector{-1.0f, 1.0f, 0.0f, 1.0f};

	// check that the 2d origin 0,0 translates to the topleft corner -1,-1
	odMatrix_multiply_vector_3d(&matrix, &vector);
	OD_ASSERT(odVector_equals(&vector, &expected_vector));

	// check that the matrix end translates to the bottom right corner 1,1
	vector = odVector{projection_width, projection_height, 0.0f, 1.0f};
	expected_vector = odVector{1.0f, -1.0f, 0.0f, 1.0f};
	odMatrix_multiply_vector_3d(&matrix, &vector);
	OD_ASSERT(odVector_equals(&vector, &expected_vector));

	// check that the matrix midpoint translates to the center 0,0
	vector = odVector{projection_width / 2.0f, projection_height / 2.0f, 0.0f, 1.0f};
	expected_vector = odVector{0.0f, 0.0f, 0.0f, 1.0f};
	odMatrix_multiply_vector_3d(&matrix, &vector);
	OD_ASSERT(odVector_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix_multiply_scale) {
	odMatrix matrix = *odMatrix_get_identity();
	odVector scale1{2.0f, 3.0f, 4.0f, 1.0f};
	odMatrix_scale_3d(&matrix, scale1.x, scale1.y, scale1.z);

	odMatrix matrix2 = *odMatrix_get_identity();
	odVector scale2{5.0f, 6.0f, 7.0f, 1.0f};
	odMatrix_scale_3d(&matrix2, scale2.x, scale2.y, scale2.z);

	odMatrix_multiply(&matrix, &matrix2);

	odVector vector{1.0f, 1.0f, 1.0f, 1.0f};
	odMatrix_multiply_vector_3d(&matrix, &vector);
	odVector expected_vector{scale1.x * scale2.x, scale1.y * scale2.y, scale1.z * scale2.z, 1.0f};
	OD_ASSERT(odVector_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix_multiply_translate_3d) {
	odMatrix matrix = *odMatrix_get_identity();
	odVector translate1{2.0f, 3.0f, -4.0f, 1.0f};
	odMatrix_translate_3d(&matrix, translate1.x, translate1.y, translate1.z);

	odMatrix matrix2 = *odMatrix_get_identity();
	odVector translate2{5.0f, -6.0f, 7.0f, 1.0f};
	odMatrix_translate_3d(&matrix2, translate2.x, translate2.y, translate2.z);
	odMatrix_multiply(&matrix, &matrix2);

	odMatrix matrix3 = *odMatrix_get_identity();
	odVector translate3{-0.0f, 2e+23f, -2e-4f, 1.0f};
	odMatrix_translate_3d(&matrix3, translate3.x, translate3.y, translate3.z);	
	odMatrix_multiply(&matrix, &matrix3);

	odVector translate0{-6.0f, 4.0f, -6e5f, 1.0f};
	odVector vector = translate0;
	odMatrix_multiply_vector_3d(&matrix, &vector);
	odVector expected_vector{
		translate0.x + translate1.x + translate2.x + translate3.x,
		translate0.y + translate1.y + translate2.y + translate3.y,
		translate0.z + translate1.z + translate2.z + translate3.z,
		1.0f};
	OD_ASSERT(odVector_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix_multiply_scale_translate_3d) {
	float scale1 = 4.0f;
	float translate1 = 0.08f;
	odMatrix matrix = *odMatrix_get_identity();
	odMatrix_init(&matrix, scale1, scale1, scale1, translate1, translate1, translate1);

	float scale2 = -3.0f;
	float translate2 = -2e3f;
	odMatrix matrix2 = *odMatrix_get_identity();
	odMatrix_init(&matrix2, scale2, scale2, scale2, translate2, translate2, translate2);
	odMatrix_multiply(&matrix, &matrix2);

	float scale0 = 3.0f;
	odVector vector{scale0, scale0, scale0, 1.0f};
	odMatrix_multiply_vector_3d(&matrix, &vector);
	float expected_value = (scale0 * scale1 * scale2) + translate1 + (scale1 * translate2);
	odVector expected_vector{expected_value, expected_value, expected_value, 1.0f};
	OD_ASSERT(odVector_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix_multiply_scale_translate_rotate_clockwise_2d) {
	float scale1 = 0.789f;
	float translate1 = 6.789f;
	odMatrix matrix = *odMatrix_get_identity();
	odMatrix_init(&matrix, scale1, scale1, 1.0f, translate1, translate1, 0.0f);

	odVector vector{1.0f, 0.0f, 0.0f, 1.0f};
	odMatrix_multiply_vector_3d(&matrix, &vector);
	odVector expected_vector{scale1 + translate1, translate1, 0.0f, 1.0f};
	OD_ASSERT(odVector_equals(&vector, &expected_vector));

	odMatrix_rotate_z_3d(&matrix, 180.0f);

	vector = odVector{1.0f, 1.0f, 0.0f, 1.0f};
	odMatrix_multiply_vector_3d(&matrix, &vector);
	expected_vector = odVector{-scale1 + translate1, -scale1 + translate1, 0.0f, 1.0f};
	OD_ASSERT(odVector_epsilon_equals(&vector, &expected_vector));

	odMatrix_init(&matrix, scale1, scale1, 1.0f, translate1, translate1, 0.0f);
	odMatrix_rotate_z_3d(&matrix, 90.0f);
	vector = odVector{1.0f, 1.0f, 0.0f, 1.0f};
	odMatrix_multiply_vector_3d(&matrix, &vector);
	expected_vector = odVector{scale1 + translate1, -scale1 + translate1, 0.0f, 1.0f};
	OD_ASSERT(odVector_epsilon_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix_multiply_vector_3d) {
	odMatrix matrix{};
	odVector vector{};
	odVector expected_vector{};

	for (float translate = -4.0f; translate <= 4.0f; translate++) {
		for (float scale = -4.0f; scale < 4.0f; scale++) {
			float translate_scale = scale + translate;
			odMatrix_init(&matrix, scale, scale, scale, translate, translate, translate);
			vector = odVector{1.0f, 1.0f, 1.0f, 1.0f};
			expected_vector = odVector{translate_scale, translate_scale, translate_scale, 1.0f};
			odMatrix_multiply_vector_3d(&matrix, &vector);
			OD_ASSERT(odVector_equals(&vector, &expected_vector));

			vector = odVector{0.0f, 0.0f, 0.0f, 1.0f};
			expected_vector = odVector{translate, translate, translate, 1.0f};
			odMatrix_multiply_vector_3d(&matrix, &vector);
			OD_ASSERT(odVector_equals(&vector, &expected_vector));
		}
	}
}
OD_TEST(odTest_odMatrix_multiply_vector_3d_with_indentity) {
	odMatrix matrix = *odMatrix_get_identity();
	odVector vector{-3.0f, 6.0f, 2e5f, 1.0f};
	odVector expected_vector = vector;
	odMatrix_multiply_vector_3d(&matrix, &vector);
	OD_ASSERT(odVector_equals(&vector, &expected_vector));
}


OD_TEST_SUITE(
	odTestSuite_odMatrix,
	odTest_odMatrix_init,
	odTest_odMatrix_init_ortho_2d,
	odTest_odMatrix_multiply_scale,
	odTest_odMatrix_multiply_translate_3d,
	odTest_odMatrix_multiply_scale_translate_3d,
	odTest_odMatrix_multiply_scale_translate_rotate_clockwise_2d,
	odTest_odMatrix_multiply_vector_3d,
	odTest_odMatrix_multiply_vector_3d_with_indentity,
)
