#include <od/core/matrix.h>

#include <cmath>
#include <cstring>

#include <od/core/debug.h>
#include <od/core/math.h>
#include <od/core/vector.h>

const char* odMatrix_get_debug_string(const odMatrix* matrix) {
	if (matrix == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"\"[%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g]\"",
		static_cast<double>(matrix->matrix[0]),
		static_cast<double>(matrix->matrix[1]),
		static_cast<double>(matrix->matrix[2]),
		static_cast<double>(matrix->matrix[3]),
		static_cast<double>(matrix->matrix[4]),
		static_cast<double>(matrix->matrix[5]),
		static_cast<double>(matrix->matrix[6]),
		static_cast<double>(matrix->matrix[7]),
		static_cast<double>(matrix->matrix[8]),
		static_cast<double>(matrix->matrix[9]),
		static_cast<double>(matrix->matrix[10]),
		static_cast<double>(matrix->matrix[11]),
		static_cast<double>(matrix->matrix[12]),
		static_cast<double>(matrix->matrix[13]),
		static_cast<double>(matrix->matrix[14]),
		static_cast<double>(matrix->matrix[15]));
}
bool odMatrix_check_valid(const odMatrix* matrix) {
	if (!OD_CHECK(matrix != nullptr)) {
		return false;
	}

	for (uint32_t i = 0; i < OD_MATRIX_ELEM_COUNT; i++) {
		if (!OD_CHECK(std::isfinite(matrix->matrix[i]))) {
			return false;
		}
	}

	return true;
}
bool odMatrix_check_valid_3d(const odMatrix* matrix) {
	if (!OD_CHECK(odMatrix_check_valid(matrix))
		|| !OD_CHECK(matrix->matrix[3] == 0.0f)
		|| !OD_CHECK(matrix->matrix[7] == 0.0f)
		|| !OD_CHECK(matrix->matrix[11] == 0.0f)
		|| !OD_CHECK((matrix->matrix[15] == 0.0f) || (matrix->matrix[15] == 1.0f))) {
		return false;
	}

	return true;
}
void odMatrix_init_3d(odMatrix* matrix,
					  float scale_x, float scale_y, float scale_z,
					  float translate_x, float translate_y, float translate_z) {
	if (!OD_DEBUG_CHECK(matrix != nullptr)
		|| !OD_DEBUG_CHECK(std::isfinite(scale_x))
		|| !OD_DEBUG_CHECK(std::isfinite(scale_y))
		|| !OD_DEBUG_CHECK(std::isfinite(scale_z))
		|| !OD_DEBUG_CHECK(std::isfinite(translate_x))
		|| !OD_DEBUG_CHECK(std::isfinite(translate_y))
		|| !OD_DEBUG_CHECK(std::isfinite(translate_z))) {
		return;
	}

	// column-major
	*matrix = odMatrix{{
		scale_x, 0.0f, 0.0f, 0.0f,
		0.0f, scale_y, 0.0f, 0.0f,
		0.0f, 0.0f, scale_z, 0.0f,
		translate_x, translate_y, translate_z, 1.0f
	}};
	OD_DISCARD(OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix)));
}
void odMatrix_init_ortho_2d(odMatrix* matrix, int32_t width, int32_t height) {
	if (!OD_DEBUG_CHECK(matrix != nullptr)
		|| !OD_DEBUG_CHECK((width > 0) && (width <= OD_FLOAT_PRECISE_INT_MAX))
		|| !OD_DEBUG_CHECK((height > 0) && (height <= OD_FLOAT_PRECISE_INT_MAX))) {
		return;
	}

	// transform from 2d view space to clip space:
	// from x=0..view_width, y=0..view_height
	// to x=-1..1, y=-1..1
	// it also flips y, so (0,0)=top left, (0,height)=bottom left
	return odMatrix_init_3d(
		matrix,

		// scales xy from 0..width,0..height to 0..2,0..2
		2.0f / static_cast<float>(width),
		-2.0f / static_cast<float>(height),  // flipped y so y is down
		1.0f / OD_FLOAT_PRECISE_INT_MAX,

		// translates x and y from 0..2 (output from scaling) to -1..1
		-1.0f,
		1.0f,  // flipped y so y is down
		0.0f
	);
}
void odMatrix_multiply(odMatrix* matrix, const odMatrix* other) {
	if (!OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix))
		|| !OD_DEBUG_CHECK(odMatrix_check_valid_3d(other))) {
		return;
	}

	const float* a = matrix->matrix;
	const float* b = other->matrix;

	*matrix = odMatrix{{
		((a[0] * b[0])
		 + (a[4] * b[1])
		 + (a[8]  * b[2])
		 + (a[12] * b[3])),
		((a[1] * b[0])
		 + (a[5] * b[1])
		 + (a[9]  * b[2])
		 + (a[13] * b[3])),
		((a[2] * b[0])
		 + (a[6] * b[1])
		 + (a[10] * b[2])
		 + (a[14] * b[3])),
		((a[3] * b[0])
		 + (a[7] * b[1])
		 + (a[11] * b[2])
		 + (a[15] * b[3])),
		((a[0] * b[4])
		 + (a[4] * b[5])
		 + (a[8]  * b[6])
		 + (a[12] * b[7])),
		((a[1] * b[4])
		 + (a[5] * b[5])
		 + (a[9]  * b[6])
		 + (a[13] * b[7])),
		((a[2] * b[4])
		 + (a[6] * b[5])
		 + (a[10] * b[6])
		 + (a[14] * b[7])),
		((a[3] * b[4])
		 + (a[7] * b[5])
		 + (a[11] * b[6])
		 + (a[15] * b[7])),
		((a[0] * b[8])
		 + (a[4] * b[9])
		 + (a[8]  * b[10])
		 + (a[12] * b[11])),
		((a[1] * b[8])
		 + (a[5] * b[9])
		 + (a[9]  * b[10])
		 + (a[13] * b[11])),
		((a[2] * b[8])
		 + (a[6] * b[9])
		 + (a[10] * b[10])
		 + (a[14] * b[11])),
		((a[3] * b[8])
		 + (a[7] * b[9])
		 + (a[11] * b[10])
		 + (a[15] * b[11])),
		((a[0] * b[12])
		 + (a[4] * b[13])
		 + (a[8]  * b[14])
		 + (a[12] * b[15])),
		((a[1] * b[12])
		 + (a[5] * b[13])
		 + (a[9]  * b[14])
		 + (a[13] * b[15])),
		((a[2] * b[12])
		 + (a[6] * b[13])
		 + (a[10] * b[14])
		 + (a[14] * b[15])),
		((a[3] * b[12])
		 + (a[7] * b[13])
		 + (a[11] * b[14])
		 + (a[15] * b[15])),
	}};

	OD_DISCARD(OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix)));
}
void odMatrix_multiply_vector(const odMatrix* matrix, odVector* vector) {
	if (!OD_DEBUG_CHECK(odMatrix_check_valid(matrix))
		|| !OD_DEBUG_CHECK(odVector_check_valid(vector))) {
		return;
	}

	const float* m = matrix->matrix;

	// we intentionally leave w untouched and treat it as 1 everywhere for calculations
	*vector = odVector{
		((m[0] * vector->x)
		 + (m[4] * vector->y)
		 + (m[8]  * vector->z)
		 + (m[12] * vector->w)),
		((m[1] * vector->x)
		 + (m[5] * vector->y)
		 + (m[9]  * vector->z)
		 + (m[13] * vector->w)),
		((m[2] * vector->x)
		 + (m[6] * vector->y)
		 + (m[10] * vector->z)
		 + (m[14] * vector->w)),
		((m[3] * vector->x)
		 + (m[7] * vector->y)
		 + (m[11] * vector->z)
		 + (m[15] * vector->w)),
	};
	OD_DISCARD(OD_DEBUG_CHECK(odVector_check_valid(vector)));
}
void odMatrix_multiply_vector_3d(const odMatrix* matrix, odVector* vector) {
	if (!OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix))
		|| !OD_DEBUG_CHECK(odVector_check_valid_3d(vector))) {
		return;
	}

	vector->w = 1.0f;
	odMatrix_multiply_vector(matrix, vector);
	OD_DISCARD(OD_DEBUG_CHECK(odVector_check_valid_3d(vector)));
}
void odMatrix_scale_3d(odMatrix* matrix, float scale_x, float scale_y, float scale_z) {
	if (!OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix))
		|| !OD_DEBUG_CHECK(std::isfinite(scale_x))
		|| !OD_DEBUG_CHECK(std::isfinite(scale_y))
		|| !OD_DEBUG_CHECK(std::isfinite(scale_z))) {
		return;
	}

	odMatrix scale_matrix{{
		scale_x, 0.0f, 0.0f, 0.0f,
		0.0f, scale_y, 0.0f, 0.0f,
		0.0f, 0.0f, scale_z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	}};
	odMatrix_multiply(matrix, &scale_matrix);
	OD_DISCARD(OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix)));
}
void odMatrix_translate_3d(odMatrix* matrix, float translate_x, float translate_y, float translate_z) {
	if (!OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix))
		|| !OD_DEBUG_CHECK(std::isfinite(translate_x))
		|| !OD_DEBUG_CHECK(std::isfinite(translate_y))
		|| !OD_DEBUG_CHECK(std::isfinite(translate_z))) {
		return;
	}

	odMatrix translate_matrix{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		translate_x, translate_y, translate_z, 1.0f
	};
	odMatrix_multiply(matrix, &translate_matrix);
	OD_DISCARD(OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix)));
}
void odMatrix_rotate_z_3d(odMatrix* matrix, float rotate_clock_deg) {
	if (!OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix))
		|| !OD_DEBUG_CHECK(std::isfinite(rotate_clock_deg))) {
		return;
	}

	float rotate_rad = odFloat_deg_to_rad(rotate_clock_deg);
	float rotate_sin = sinf(rotate_rad);
	float rotate_cos = cosf(rotate_rad);

	odMatrix rotate_matrix{{
		rotate_cos, -rotate_sin, 0.0f, 0.0f,
		rotate_sin, rotate_cos, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	}};
	odMatrix_multiply(matrix, &rotate_matrix);
	OD_DISCARD(OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix)));
}
bool odMatrix_get_equals(const odMatrix* matrix1, const odMatrix* matrix2) {
	if (!OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix1))
		|| !OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix2))) {
		return false;
	}

	for (int32_t i = 0; i < OD_MATRIX_ELEM_COUNT; i++) {
		if (matrix1->matrix[i] != matrix2->matrix[i]) {
			return false;
		}
	}

	return true;
}
bool odMatrix_epsilon_get_equals(const odMatrix* matrix1, const odMatrix* matrix2) {
	if (!OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix1))
		|| !OD_DEBUG_CHECK(odMatrix_check_valid_3d(matrix2))) {
		return false;
	}

	for (int32_t i = 0; i < OD_MATRIX_ELEM_COUNT; i++) {
		if (!odFloat_epsilon_get_equals(matrix1->matrix[i], matrix2->matrix[i])) {
			return false;
		}
	}

	return true;
}
const odMatrix* odMatrix_get_identity() {
	static const odMatrix matrix{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return &matrix;
}
