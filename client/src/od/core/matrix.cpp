#include <od/core/matrix.h>

#include <cmath>
#include <cstring>

#include <od/core/debug.h>
#include <od/core/vector.h>

const char* odMatrix4_get_debug_string(const odMatrix4* matrix) {
	if (matrix == nullptr) {
		return "odMatrix4{this=nullptr}";
	}

	return odDebugString_format(
		"odMatrix4{this=%p, matrix={\n%g, %g, %g, %g,\n%g, %g, %g, %g,\n%g, %g, %g, %g,\n%g, %g, %g, %g}}",
		static_cast<const void*>(matrix),
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
void odMatrix4_init(odMatrix4* out_matrix,
				   float scale_x, float scale_y, float scale_z,
				   float translate_x, float translate_y, float translate_z) {
	if (!OD_DEBUG_CHECK(out_matrix != nullptr)) {
		return;
	}

	*out_matrix = {
		scale_x, 0, 0, 0,
		0, scale_y, 0, 0,
		0, 0, scale_z, 0,
		translate_x, translate_y, translate_z, 1
	};
}
void odMatrix4_init_view_2d(odMatrix4* out_matrix, int32_t width, int32_t height) {
	if (!OD_DEBUG_CHECK(out_matrix != nullptr)) {
		return;
	}

	return odMatrix4_init(
		out_matrix,
		2.0f / static_cast<float>(width),
		2.0f / static_cast<float>(height),
		1.0f / static_cast<float>(1 << 20),  // +/- 2^20, near the int limit for float32
		-1.0f,
		-1.0f,
		0.0f
	);
}
void odMatrix4_multiply(odMatrix4* out_matrix, odMatrix4* a, odMatrix4* b) {
	if (!OD_DEBUG_CHECK(out_matrix != nullptr)
		|| !OD_DEBUG_CHECK(a != nullptr)
		|| !OD_DEBUG_CHECK(b != nullptr)) {
		return;
	}

	const float* ma = a->matrix;
	const float* mb = b->matrix;

	*out_matrix = odMatrix4{{
		(ma[0] * mb[0])  + (ma[4] * mb[1])  + (ma[8]  * mb[2])  + (ma[12] * mb[3]),
		(ma[1] * mb[0])  + (ma[5] * mb[1])  + (ma[9]  * mb[2])  + (ma[13] * mb[3]),
		(ma[2] * mb[0])  + (ma[6] * mb[1])  + (ma[10] * mb[2])  + (ma[14] * mb[3]),
		(ma[3] * mb[0])  + (ma[7] * mb[1])  + (ma[11] * mb[2])  + (ma[15] * mb[3]),
		(ma[0] * mb[4])  + (ma[4] * mb[5])  + (ma[8]  * mb[6])  + (ma[12] * mb[7]),
		(ma[1] * mb[4])  + (ma[5] * mb[5])  + (ma[9]  * mb[6])  + (ma[13] * mb[7]),
		(ma[2] * mb[4])  + (ma[6] * mb[5])  + (ma[10] * mb[6])  + (ma[14] * mb[7]),
		(ma[3] * mb[4])  + (ma[7] * mb[5])  + (ma[11] * mb[6])  + (ma[15] * mb[7]),
		(ma[0] * mb[8])  + (ma[4] * mb[9])  + (ma[8]  * mb[10]) + (ma[12] * mb[11]),
		(ma[1] * mb[8])  + (ma[5] * mb[9])  + (ma[9]  * mb[10]) + (ma[13] * mb[11]),
		(ma[2] * mb[8])  + (ma[6] * mb[9])  + (ma[10] * mb[10]) + (ma[14] * mb[11]),
		(ma[3] * mb[8])  + (ma[7] * mb[9])  + (ma[11] * mb[10]) + (ma[15] * mb[11]),
		(ma[0] * mb[12]) + (ma[4] * mb[13]) + (ma[8]  * mb[14]) + (ma[12] * mb[15]),
		(ma[1] * mb[12]) + (ma[5] * mb[13]) + (ma[9]  * mb[14]) + (ma[13] * mb[15]),
		(ma[2] * mb[12]) + (ma[6] * mb[13]) + (ma[10] * mb[14]) + (ma[14] * mb[15]),
		(ma[3] * mb[12]) + (ma[7] * mb[13]) + (ma[11] * mb[14]) + (ma[15] * mb[15])
	}};
}
void odMatrix4_multiply_vector(odVector4* out_vector, odVector4* a, odMatrix4* b) {
	if (!OD_DEBUG_CHECK(out_vector != nullptr)
		|| !OD_DEBUG_CHECK(a != nullptr)
		|| !OD_DEBUG_CHECK(b != nullptr)) {
		return;
	}

	const float* v = a->vector;
	const float* m = b->matrix;

	*out_vector = odVector4{{
		(m[0] * v[0]) + (m[4] * v[1]) + (m[8]  * v[2]) + (m[12] * v[3]),
		(m[1] * v[0]) + (m[5] * v[1]) + (m[9]  * v[2]) + (m[13] * v[3]),
		(m[2] * v[0]) + (m[6] * v[1]) + (m[10] * v[2]) + (m[14] * v[3]),
		(m[3] * v[0]) + (m[7] * v[1]) + (m[11] * v[2]) + (m[15] * v[3])
	}};
}
const struct odMatrix4* odMatrix4_get_identity() {
	static const odMatrix4 matrix{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	return &matrix;
}
