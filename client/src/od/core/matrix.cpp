#include <od/core/matrix.h>

#include <cmath>
#include <cstring>

#include <od/core/debug.h>
#include <od/core/vector.h>

const char* odMatrix4_get_debug_string(const odMatrix4* matrix) {
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
bool odMatrix4_check_valid(const odMatrix4* matrix) {
	if (!OD_CHECK(matrix != nullptr)) {
		return false;
	}

	for (uint32_t i = 0; i < OD_MATRIX4_ELEM_COUNT; i++) {
		if (!OD_CHECK(std::isfinite(matrix->matrix[i]))) {
			return false;
		}
	}

	return true;
}
void odMatrix4_init(odMatrix4* matrix,
				   float scale_x, float scale_y, float scale_z,
				   float translate_x, float translate_y, float translate_z) {
	if (!OD_DEBUG_CHECK(matrix != nullptr)
		|| !OD_DEBUG_CHECK(std::isnormal(scale_x))
		|| !OD_DEBUG_CHECK(std::isnormal(scale_y))
		|| !OD_DEBUG_CHECK(std::isnormal(scale_z))
		|| !OD_DEBUG_CHECK(std::isfinite(translate_x))
		|| !OD_DEBUG_CHECK(std::isfinite(translate_y))
		|| !OD_DEBUG_CHECK(std::isfinite(translate_z))) {
		return;
	}

	*matrix = odMatrix4{{
		scale_x, 0, 0, 0,
		0, scale_y, 0, 0,
		0, 0, scale_z, 0,
		translate_x, translate_y, translate_z, 1
	}};
}
void odMatrix4_init_view_2d(odMatrix4* matrix, int32_t width, int32_t height) {
	if (!OD_DEBUG_CHECK(matrix != nullptr)
		|| !OD_DEBUG_CHECK(width > 0)
		|| !OD_DEBUG_CHECK(height > 0)) {
		return;
	}

	// transform from world space to view space:
	// from x=0..width, y=0..height, z=...
	// to x=-1..1, y=-1..1, z=-1..1
	// it also flips y, so (0,0)=top left, (0,height)=bottom left
	return odMatrix4_init(
		matrix,
		// coords are normally given in the in -1 to 1 range
		// we scale by 2x window size and translate by 1 to
		// translate screen coords (0, 0 at top left) into -1 to 1

		2.0f / static_cast<float>(width),
		-2.0f / static_cast<float>(height),  // flipped so y in world space starts at top of screen
		1.0f / static_cast<float>(1 << 24),  // +/- 2^24, limits of a precise integer for float32

		// translate is constant: our 2d camera pos is defined by viewport, not view matrix
		-1.0f, // 0..width before scale -> 0..2 after scale -> -1..1 after scale+translate
		1.0f,  // positive as we're scaling y by -1 so y in world space starts at top of screen
		0.0f
	);
}
void odMatrix4_scale(odMatrix4* matrix, const odVector4* vector) {
	if (!OD_DEBUG_CHECK(odVector4_check_valid(vector))
		|| !OD_DEBUG_CHECK(odMatrix4_check_valid(matrix))) {
		return;
	}

	float* m = matrix->matrix;
	const float* v = vector->vector;

	m[0] *= v[0];
	m[5] *= v[1];
	m[10] *= v[2];
}
void odMatrix4_translate(odMatrix4* matrix, const odVector4* vector) {
	if (!OD_DEBUG_CHECK(odMatrix4_check_valid(matrix))
		|| !OD_DEBUG_CHECK(odVector4_check_valid(vector))) {
		return;
	}

	float* m = matrix->matrix;
	const float* v = vector->vector;

	m[12] += v[0];
	m[13] += v[1];
	m[14] += v[2];
}
void odMatrix4_rotate_clockwise_2d(odMatrix4* matrix, float angle_rad) {
	if (!OD_DEBUG_CHECK(odMatrix4_check_valid(matrix))
		|| !OD_DEBUG_CHECK(std::isfinite(angle_rad))) {
		return;
	}

	float angle_sin = sinf(angle_rad);
	float angle_cos = cosf(angle_rad);

	odMatrix4 rotation_matrix{{
		angle_cos, -angle_sin, 0.0f, 0.0f,
		angle_sin, angle_cos, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	}};
	odMatrix4_multiply(matrix, &rotation_matrix);
}
void odMatrix4_multiply(odMatrix4* matrix, const odMatrix4* other) {
	if (!OD_DEBUG_CHECK(odMatrix4_check_valid(matrix))
		|| !OD_DEBUG_CHECK(odMatrix4_check_valid(other))) {
		return;
	}

	const float* a = matrix->matrix;
	const float* b = other->matrix;

	*matrix = odMatrix4{{
		(a[0] * b[0])  + (a[4] * b[1])  + (a[8]  * b[2])  + (a[12] * b[3]),
		(a[1] * b[0])  + (a[5] * b[1])  + (a[9]  * b[2])  + (a[13] * b[3]),
		(a[2] * b[0])  + (a[6] * b[1])  + (a[10] * b[2])  + (a[14] * b[3]),
		(a[3] * b[0])  + (a[7] * b[1])  + (a[11] * b[2])  + (a[15] * b[3]),
		(a[0] * b[4])  + (a[4] * b[5])  + (a[8]  * b[6])  + (a[12] * b[7]),
		(a[1] * b[4])  + (a[5] * b[5])  + (a[9]  * b[6])  + (a[13] * b[7]),
		(a[2] * b[4])  + (a[6] * b[5])  + (a[10] * b[6])  + (a[14] * b[7]),
		(a[3] * b[4])  + (a[7] * b[5])  + (a[11] * b[6])  + (a[15] * b[7]),
		(a[0] * b[8])  + (a[4] * b[9])  + (a[8]  * b[10]) + (a[12] * b[11]),
		(a[1] * b[8])  + (a[5] * b[9])  + (a[9]  * b[10]) + (a[13] * b[11]),
		(a[2] * b[8])  + (a[6] * b[9])  + (a[10] * b[10]) + (a[14] * b[11]),
		(a[3] * b[8])  + (a[7] * b[9])  + (a[11] * b[10]) + (a[15] * b[11]),
		(a[0] * b[12]) + (a[4] * b[13]) + (a[8]  * b[14]) + (a[12] * b[15]),
		(a[1] * b[12]) + (a[5] * b[13]) + (a[9]  * b[14]) + (a[13] * b[15]),
		(a[2] * b[12]) + (a[6] * b[13]) + (a[10] * b[14]) + (a[14] * b[15]),
		(a[3] * b[12]) + (a[7] * b[13]) + (a[11] * b[14]) + (a[15] * b[15])
	}};
}
void odMatrix4_multiply_vector(const odMatrix4* matrix, odVector4* vector) {
	if (!OD_DEBUG_CHECK(odVector4_check_valid(vector))
		|| !OD_DEBUG_CHECK(odMatrix4_check_valid(matrix))) {
		return;
	}

	const float* m = matrix->matrix;
	const float* v = vector->vector;

	*vector = odVector4{{
		(m[0] * v[0]) + (m[4] * v[1]) + (m[8]  * v[2]) + (m[12] * v[3]),
		(m[1] * v[0]) + (m[5] * v[1]) + (m[9]  * v[2]) + (m[13] * v[3]),
		(m[2] * v[0]) + (m[6] * v[1]) + (m[10] * v[2]) + (m[14] * v[3]),
		(m[3] * v[0]) + (m[7] * v[1]) + (m[11] * v[2]) + (m[15] * v[3])
	}};
}
bool odMatrix4_equals(const odMatrix4* matrix1, const odMatrix4* matrix2) {
	if (!OD_DEBUG_CHECK(odMatrix4_check_valid(matrix1))
		|| !OD_DEBUG_CHECK(odMatrix4_check_valid(matrix2))) {
		return false;
	}

	for (int32_t i = 0; i < OD_MATRIX4_ELEM_COUNT; i++) {
		if (matrix1->matrix[i] != matrix2->matrix[i]) {
			return false;
		}
	}

	return true;
}
const odMatrix4* odMatrix4_get_identity() {
	static const odMatrix4 matrix{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	return &matrix;
}
