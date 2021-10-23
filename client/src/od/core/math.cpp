#include <od/core/math.h>

#include <od/core/debug.h>

odTransform odTransform_create(
	float scale_x,
	float scale_y,
	float scale_z,
	float translate_x,
	float translate_y,
	float translate_z) {
	return {
		scale_x, 0, 0, 0,
		0, scale_y, 0, 0,
		0, 0, scale_z, 0,
		translate_x, translate_y, translate_z, 1
	};
}
odTransform odTransform_create_view_transform(int32_t width, int32_t height) {
	return odTransform_create(
		2.0f / static_cast<float>(width),
		2.0f / static_cast<float>(height),
		1.0f / static_cast<float>(1 << 20),  // +/- 2^20, near the int limit for float32
		-1.0f,
		-1.0f,
		0.0f
	);
}
odTransform odTransform_multiply(odTransform a, odTransform b) {
	const float* ma = a.matrix;
	const float* mb = b.matrix;

	return odTransform{{
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
odVector odTransform_multiply_vector(odVector a, odTransform b) {
	const float* v = a.vector;
	const float* m = b.matrix;

	return odVector{{
		(m[0] * v[0]) + (m[4] * v[1]) + (m[8]  * v[2]) + (m[12] * v[3]),
		(m[1] * v[0]) + (m[5] * v[1]) + (m[9]  * v[2]) + (m[13] * v[3]),
		(m[2] * v[0]) + (m[6] * v[1]) + (m[10] * v[2]) + (m[14] * v[3]),
		(m[3] * v[0]) + (m[7] * v[1]) + (m[11] * v[2]) + (m[15] * v[3])
	}};
}
