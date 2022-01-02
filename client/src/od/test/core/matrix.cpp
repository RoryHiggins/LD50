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

	odVector vector{1.0f, 1.0f, 1.0f, 0.0f};
	odVector expected_vector{scale + translation, scale + translation, scale + translation, 0.0f};
	odMatrix_multiply_vector(&matrix, &vector);
	OD_ASSERT(odVector_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix_init_ortho_2d) {
	const int32_t projection_width = 256;
	const int32_t projection_height = 256;

	odMatrix matrix{};
	odMatrix_init_ortho_2d(&matrix, projection_width, projection_height);

	odVector vector{0.0f, 0.0f, 0.0f, 0.0f};
	odVector expected_vector{-1.0f, 1.0f, 0.0f, 0.0f};

	// check that the 2d origin 0,0 translates to the topleft corner -1,-1
	odMatrix_multiply_vector(&matrix, &vector);
	OD_ASSERT(odVector_equals(&vector, &expected_vector));
	
	// check that the matrix end translates to the bottom right corner 1,1
	vector = {projection_width, projection_height, 0.0f, 0.0f};
	expected_vector = {1.0f, -1.0f, 0.0f, 0.0f};
	odMatrix_multiply_vector(&matrix, &vector);
	OD_ASSERT(odVector_equals(&vector, &expected_vector));

	// check that the matrix midpoint translates to the center 0,0
	vector = {projection_width / 2.0f, projection_height / 2.0f, 0.0f, 0.0f};
	expected_vector = {0.0f, 0.0f, 0.0f, 0.0f};
	odMatrix_multiply_vector(&matrix, &vector);
	OD_ASSERT(odVector_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix_multiply_scale) {
	odMatrix matrix = *odMatrix_get_identity();
	odVector scale1{2.0f, 3.0f, 4.0f, 0.0f};
	odMatrix_scale(&matrix, scale1.vector[0], scale1.vector[1], scale1.vector[2]);

	odMatrix matrix2 = *odMatrix_get_identity();
	odVector scale2{5.0f, 6.0f, 7.0f, 0.0f};
	odMatrix_scale(&matrix2, scale2.vector[0], scale2.vector[1], scale2.vector[2]);

	odMatrix_multiply(&matrix, &matrix2);

	odVector vector{1.0f, 1.0f, 1.0f, 0.0f};
	odMatrix_multiply_vector(&matrix, &vector);
	odVector expected_vector{0.0f, 0.0f, 0.0f, 0.0f};
	for (int32_t i = 0; i < 3; i++) {
		expected_vector.vector[i] = scale1.vector[i] * scale2.vector[i];
	}
	OD_ASSERT(odVector_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix_multiply_translate_3d) {
	odMatrix matrix = *odMatrix_get_identity();
	odVector translate1{2.0f, 3.0f, -4.0f, 0.0f};
	odMatrix_translate(&matrix, translate1.vector[0], translate1.vector[1], translate1.vector[2]);

	odMatrix matrix2 = *odMatrix_get_identity();
	odVector translate2{5.0f, -6.0f, 7.0f, 0.0f};
	odMatrix_translate(&matrix2, translate2.vector[0], translate2.vector[1], translate2.vector[2]);
	odMatrix_multiply(&matrix, &matrix2);

	odMatrix matrix3 = *odMatrix_get_identity();
	odVector translate3{-0.0f, 2e+23f, -2e-4f, 0.0f};
	odMatrix_translate(&matrix3, translate3.vector[0], translate3.vector[1], translate3.vector[2]);	
	odMatrix_multiply(&matrix, &matrix3);

	odVector translate0{-6.0f, 4.0f, -6e5f, 0.0f};
	odVector vector = translate0;
	odMatrix_multiply_vector(&matrix, &vector);
	odVector expected_vector{0.0f, 0.0f, 0.0f, 0.0f};
	for (int32_t i = 0; i < 3; i++) {
		expected_vector.vector[i] = 
			(translate0.vector[i] + translate1.vector[i] + translate2.vector[i] + translate3.vector[i]);
	}
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
	odVector vector{scale0, scale0, scale0, 0.0f};
	odMatrix_multiply_vector(&matrix, &vector);
	odVector expected_vector{0.0f, 0.0f, 0.0f, 0.0f};
	for (int32_t i = 0; i < 3; i++) {
		expected_vector.vector[i] = (scale0 * scale1 * scale2) + translate1 + (scale1 * translate2);
	}
	OD_ASSERT(odVector_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix_multiply_scale_translate_rotate_clockwise_2d) {
	float scale1 = 0.789f;
	float translate1 = 6.789f;
	odMatrix matrix = *odMatrix_get_identity();
	odMatrix_init(&matrix, scale1, scale1, 1.0f, translate1, translate1, 0.0);

	odVector vector{1.0f, 0.0f, 0.0f, 0.0f};
	odMatrix_multiply_vector(&matrix, &vector);
	odVector expected_vector{scale1 + translate1, translate1, 0.0f, 0.0f};
	OD_ASSERT(odVector_equals(&vector, &expected_vector));

	odMatrix_rotate_z(&matrix, 180.0f);

	vector = {1.0f, 1.0f, 0.0f, 0.0f};
	odMatrix_multiply_vector(&matrix, &vector);
	expected_vector = {-scale1 + translate1, -scale1 + translate1, 0.0f, 0.0f};
	OD_ASSERT(odVector_epsilon_equals(&vector, &expected_vector));

	odMatrix_init(&matrix, scale1, scale1, 1.0f, translate1, translate1, 0.0);
	odMatrix_rotate_z(&matrix, 90.0f);
	vector = {1.0f, 1.0f, 0.0f, 0.0f};
	odMatrix_multiply_vector(&matrix, &vector);
	expected_vector = {scale1 + translate1, -scale1 + translate1, 0.0f, 0.0f};
	OD_ASSERT(odVector_epsilon_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix_multiply_vector) {
	odMatrix matrix{};
	odVector vector{};
	odVector expected_vector{};

	for (float translate = -4.0f; translate <= 4.0f; translate++) {
		for (float scale = -4.0f; scale < 4.0f; scale++) {
			float translate_scale = scale + translate;
			odMatrix_init(&matrix, scale, scale, scale, translate, translate, translate);
			vector = {1.0f, 1.0f, 1.0f, 0.0f};
			expected_vector = {translate_scale, translate_scale, translate_scale, 0.0f};
			odMatrix_multiply_vector(&matrix, &vector);
			OD_ASSERT(odVector_equals(&vector, &expected_vector));

			vector = {0.0f, 0.0f, 0.0f, 0.0f};
			expected_vector = {translate, translate, translate, 0.0f};
			odMatrix_multiply_vector(&matrix, &vector);
			OD_ASSERT(odVector_equals(&vector, &expected_vector));
		}
	}
}
OD_TEST(odTest_odMatrix_multiply_vector_with_indentity) {
	odMatrix matrix = *odMatrix_get_identity();
	odVector vector{-3.0, 6.0, 2e5f, 0.0};
	odVector expected_vector = vector;
	odMatrix_multiply_vector(&matrix, &vector);
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
	odTest_odMatrix_multiply_vector,
	odTest_odMatrix_multiply_vector_with_indentity,
)
