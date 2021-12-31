#include <od/core/matrix.h>
#include <od/core/vector.h>
#include <od/test/test.hpp>

// TODO remove
#include <cfloat>
#include <od/core/math.h>

OD_TEST(odTest_odMatrix4f_init_transform_3d) {
	odMatrix4f matrix{};
	odMatrix4f_init_transform_3d(&matrix, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f);

	OD_ASSERT(odMatrix4f_equals(&matrix, odMatrix4f_get_identity()));

	float translation = 4.0f;
	float scale = 3.0f;
	odMatrix4f_init_transform_3d(&matrix, scale, scale, scale, translation, translation, translation);

	odVector4f vector{1.0f, 1.0f, 1.0f, 1.0f};
	odVector4f expected_vector{scale + translation, scale + translation, scale + translation, 1};
	odMatrix4f_multiply_vector_3d(&matrix, &vector);
	OD_ASSERT(odVector4f_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix4f_init_ortho_2d) {
	const int32_t projection_width = 256;
	const int32_t projection_height = 256;

	odMatrix4f matrix{};
	odMatrix4f_init_ortho_2d(&matrix, projection_width, projection_height);

	odVector4f vector{0.0f, 0.0f, 0.0f, 1.0f};
	odVector4f expected_vector{-1.0f, 1.0f, 0.0f, 1.0f};

	// check that the 2d origin 0,0 translates to the topleft corner -1,-1
	odMatrix4f_multiply_vector_3d(&matrix, &vector);
	OD_ASSERT(odVector4f_equals(&vector, &expected_vector));
	
	// check that the matrix end translates to the bottom right corner 1,1
	vector = {projection_width, projection_height, 0.0f, 1.0f};
	expected_vector = {1.0f, -1.0f, 0.0f, 1.0f};
	odMatrix4f_multiply_vector_3d(&matrix, &vector);
	OD_ASSERT(odVector4f_equals(&vector, &expected_vector));

	// check that the matrix midpoint translates to the center 0,0
	vector = {projection_width / 2.0f, projection_height / 2.0f, 0.0f, 1.0f};
	expected_vector = {0.0f, 0.0f, 0.0f, 1.0f};
	odMatrix4f_multiply_vector_3d(&matrix, &vector);
	OD_ASSERT(odVector4f_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix4f_multiply_scale) {
	odMatrix4f matrix = *odMatrix4f_get_identity();
	odVector4f scale1{2.0f, 3.0f, 4.0f, 1.0f};
	odMatrix4f_scale_3d(&matrix, scale1.vector[0], scale1.vector[1], scale1.vector[2]);

	odMatrix4f matrix2 = *odMatrix4f_get_identity();
	odVector4f scale2{5.0f, 6.0f, 7.0f, 1.0f};
	odMatrix4f_scale_3d(&matrix2, scale2.vector[0], scale2.vector[1], scale2.vector[2]);

	odMatrix4f_multiply(&matrix, &matrix2);

	odVector4f vector{1.0f, 1.0f, 1.0f, 1.0f};
	odMatrix4f_multiply_vector_3d(&matrix, &vector);
	odVector4f expected_vector{0.0f, 0.0f, 0.0f, 1.0f};
	for (int32_t i = 0; i < 3; i++) {
		expected_vector.vector[i] = scale1.vector[i] * scale2.vector[i];
	}
	OD_ASSERT(odVector4f_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix4f_multiply_translate_3d) {
	odMatrix4f matrix = *odMatrix4f_get_identity();
	odVector4f translate1{2.0f, 3.0f, -4.0f, 1.0f};
	odMatrix4f_translate_3d(&matrix, translate1.vector[0], translate1.vector[1], translate1.vector[2]);

	odMatrix4f matrix2 = *odMatrix4f_get_identity();
	odVector4f translate2{5.0f, -6.0f, 7.0f, 1.0f};
	odMatrix4f_translate_3d(&matrix2, translate2.vector[0], translate2.vector[1], translate2.vector[2]);
	odMatrix4f_multiply(&matrix, &matrix2);

	odMatrix4f matrix3 = *odMatrix4f_get_identity();
	odVector4f translate3{-0.0f, 2e+23f, -2e-4f, 1.0f};
	odMatrix4f_translate_3d(&matrix3, translate3.vector[0], translate3.vector[1], translate3.vector[2]);	
	odMatrix4f_multiply(&matrix, &matrix3);

	odVector4f translate0{-6.0f, 4.0f, -6e5f, 1.0f};
	odVector4f vector = translate0;
	odMatrix4f_multiply_vector_3d(&matrix, &vector);
	odVector4f expected_vector{0.0f, 0.0f, 0.0f, 1.0f};
	for (int32_t i = 0; i < 3; i++) {
		expected_vector.vector[i] = 
			(translate0.vector[i] + translate1.vector[i] + translate2.vector[i] + translate3.vector[i]);
	}
	OD_ASSERT(odVector4f_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix4f_multiply_scale_translate_3d) {
	float scale1 = 4.0f;
	float translate1 = 0.08f;
	odMatrix4f matrix = *odMatrix4f_get_identity();
	odMatrix4f_init_transform_3d(&matrix, scale1, scale1, scale1, translate1, translate1, translate1);

	float scale2 = -3.0f;
	float translate2 = -2e3f;
	odMatrix4f matrix2 = *odMatrix4f_get_identity();
	odMatrix4f_init_transform_3d(&matrix2, scale2, scale2, scale2, translate2, translate2, translate2);
	odMatrix4f_multiply(&matrix, &matrix2);

	float scale0 = 3.0f;
	odVector4f vector{scale0, scale0, scale0, 1.0f};
	odMatrix4f_multiply_vector_3d(&matrix, &vector);
	odVector4f expected_vector{0.0f, 0.0f, 0.0f, 1.0f};
	for (int32_t i = 0; i < 3; i++) {
		expected_vector.vector[i] = (scale0 * scale1 * scale2) + translate1 + (scale1 * translate2);
	}
	OD_ASSERT(odVector4f_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix4f_multiply_scale_translate_rotate_clockwise_2d) {
	float scale1 = 0.789f;
	float translate1 = 6.789f;
	odMatrix4f matrix = *odMatrix4f_get_identity();
	odMatrix4f_init_transform_3d(&matrix, scale1, scale1, 1.0f, translate1, translate1, 0.0);

	odVector4f vector{1.0f, 0.0f, 0.0f, 1.0f};
	odMatrix4f_multiply_vector_2d(&matrix, &vector);
	odVector4f expected_vector{scale1 + translate1, translate1, 0.0f, 1.0f};
	OD_ASSERT(odVector4f_equals(&vector, &expected_vector));

	odMatrix4f_rotate_2d(&matrix, 180.0f);

	vector = {1.0f, 1.0f, 0.0f, 1.0f};
	odMatrix4f_multiply_vector_2d(&matrix, &vector);
	expected_vector = {-scale1 + translate1, -scale1 + translate1, 0.0f, 1.0f};
	OD_ASSERT(odVector4f_epsilon_equals(&vector, &expected_vector));

	odMatrix4f_init_transform_3d(&matrix, scale1, scale1, 1.0f, translate1, translate1, 0.0);
	odMatrix4f_rotate_2d(&matrix, 90.0f);
	vector = {1.0f, 1.0f, 0.0f, 1.0f};
	odMatrix4f_multiply_vector_2d(&matrix, &vector);
	expected_vector = {scale1 + translate1, -scale1 + translate1, 0.0f, 1.0f};
	// OD_INFO("%s", odMatrix4f_get_debug_string(&matrix));
	// OD_INFO("%s", odVector4f_get_debug_string(&vector));
	// OD_INFO("%s", odVector4f_get_debug_string(&expected_vector));
	OD_ASSERT(odVector4f_epsilon_equals(&vector, &expected_vector));
}
OD_TEST(odTest_odMatrix4f_multiply_vector_3d) {
	odMatrix4f matrix{};
	odVector4f vector{};
	odVector4f expected_vector{};

	for (float translate = -4.0f; translate <= 4.0f; translate++) {
		for (float scale = -4.0f; scale < 4.0f; scale++) {
			float translate_scale = scale + translate;
			odMatrix4f_init_transform_3d(&matrix, scale, scale, scale, translate, translate, translate);
			odVector4f_init_3d(&vector, 1.0f, 1.0f, 1.0f);
			odVector4f_init_3d(&expected_vector, translate_scale, translate_scale, translate_scale);
			odMatrix4f_multiply_vector_3d(&matrix, &vector);
			OD_ASSERT(odVector4f_equals(&vector, &expected_vector));

			odVector4f_init_3d(&vector, 0.0f, 0.0f, 0.0f);
			odVector4f_init_3d(&expected_vector, translate, translate, translate);
			odMatrix4f_multiply_vector_3d(&matrix, &vector);
			OD_ASSERT(odVector4f_equals(&vector, &expected_vector));
		}
	}
}
OD_TEST(odTest_odMatrix4f_multiply_vector_3d_with_indentity) {
	odMatrix4f matrix = *odMatrix4f_get_identity();
	odVector4f vector{-3.0, 6.0, 2e5f, 1.0};
	odVector4f expected_vector = vector;
	odMatrix4f_multiply_vector_3d(&matrix, &vector);
	OD_ASSERT(odVector4f_equals(&vector, &expected_vector));
}


OD_TEST_SUITE(
	odTestSuite_odMatrix,
	odTest_odMatrix4f_init_transform_3d,
	odTest_odMatrix4f_init_ortho_2d,
	odTest_odMatrix4f_multiply_scale,
	odTest_odMatrix4f_multiply_translate_3d,
	odTest_odMatrix4f_multiply_scale_translate_3d,
	odTest_odMatrix4f_multiply_scale_translate_rotate_clockwise_2d,
	odTest_odMatrix4f_multiply_vector_3d,
	odTest_odMatrix4f_multiply_vector_3d_with_indentity,
)
