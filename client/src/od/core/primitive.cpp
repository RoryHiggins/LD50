#include <od/core/primitive.h>

#include <cstring>
#include <cinttypes>

#include <od/core/debug.h>


const char* odBounds_get_debug_string(const odBounds* bounds) {
	if (bounds == nullptr) {
		return "odBounds{this=nullptr}";
	}

	return odDebugString_format(
		"odBounds{this=%p, x=%f, y=%f, width=%f, height=%f}",
		static_cast<const void*>(bounds),
		static_cast<double>(bounds->x),
		static_cast<double>(bounds->y),
		static_cast<double>(bounds->width),
		static_cast<double>(bounds->height));
}

const char* odColor_get_debug_string(const odColor* color) {
	if (color == nullptr) {
		return "odColor{this=nullptr}";
	}

	return odDebugString_format(
		"odColor{this=%p, r=%" PRIu8 ", g=%" PRIu8 ", b=%" PRIu8 ", a=%" PRIu8 "}",
		static_cast<const void*>(color),
		color->r,
		color->g,
		color->b,
		color->a);
}

const char* odVector_get_debug_string(const odVector* vector) {
	if (vector == nullptr) {
		return "odVector{this=nullptr}";
	}

	return odDebugString_format(
		"odVector{this=%p, vector={%f, %f, %f, %f}}",
		static_cast<const void*>(vector),
		static_cast<double>(vector->vector[0]),
		static_cast<double>(vector->vector[1]),
		static_cast<double>(vector->vector[2]),
		static_cast<double>(vector->vector[3]));
}

const char* odTransform_get_debug_string(const odTransform* transform) {
	if (transform == nullptr) {
		return "odTransform{this=nullptr}";
	}

	return odDebugString_format(
		"odTransform{this=%p, transform={\n%f, %f, %f, %f,\n%f, %f, %f, %f,\n%f, %f, %f, %f,\n%f, %f, %f, %f}}",
		static_cast<const void*>(transform),
		static_cast<double>(transform->matrix[0]),
		static_cast<double>(transform->matrix[1]),
		static_cast<double>(transform->matrix[2]),
		static_cast<double>(transform->matrix[3]),
		static_cast<double>(transform->matrix[4]),
		static_cast<double>(transform->matrix[5]),
		static_cast<double>(transform->matrix[6]),
		static_cast<double>(transform->matrix[7]),
		static_cast<double>(transform->matrix[8]),
		static_cast<double>(transform->matrix[9]),
		static_cast<double>(transform->matrix[10]),
		static_cast<double>(transform->matrix[11]),
		static_cast<double>(transform->matrix[12]),
		static_cast<double>(transform->matrix[13]),
		static_cast<double>(transform->matrix[14]),
		static_cast<double>(transform->matrix[15]));
}
void odTransform_init(odTransform* out_transform,
					  float scale_x, float scale_y, float scale_z,
					  float translate_x, float translate_y, float translate_z) {
	if (!OD_DEBUG_CHECK(out_transform != nullptr)) {
		return;
	}

	*out_transform = {
		scale_x, 0, 0, 0,
		0, scale_y, 0, 0,
		0, 0, scale_z, 0,
		translate_x, translate_y, translate_z, 1
	};
}
void odTransform_init_view_transform(odTransform* out_transform, int32_t width, int32_t height) {
	if (!OD_DEBUG_CHECK(out_transform != nullptr)) {
		return;
	}

	return odTransform_init(
		out_transform,
		2.0f / static_cast<float>(width),
		2.0f / static_cast<float>(height),
		1.0f / static_cast<float>(1 << 20),  // +/- 2^20, near the int limit for float32
		-1.0f,
		-1.0f,
		0.0f
	);
}
void odTransform_multiply(odTransform* out_transform, odTransform* a, odTransform* b) {
	if (!OD_DEBUG_CHECK(out_transform != nullptr)
		|| !OD_DEBUG_CHECK(a != nullptr)
		|| !OD_DEBUG_CHECK(b != nullptr)) {
		return;
	}

	const float* ma = a->matrix;
	const float* mb = b->matrix;

	*out_transform = odTransform{{
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
void odTransform_multiply_vector(odVector* out_vector, odVector* a, odTransform* b) {
	if (!OD_DEBUG_CHECK(out_vector != nullptr)
		|| !OD_DEBUG_CHECK(a != nullptr)
		|| !OD_DEBUG_CHECK(b != nullptr)) {
		return;
	}

	const float* v = a->vector;
	const float* m = b->matrix;

	*out_vector = odVector{{
		(m[0] * v[0]) + (m[4] * v[1]) + (m[8]  * v[2]) + (m[12] * v[3]),
		(m[1] * v[0]) + (m[5] * v[1]) + (m[9]  * v[2]) + (m[13] * v[3]),
		(m[2] * v[0]) + (m[6] * v[1]) + (m[10] * v[2]) + (m[14] * v[3]),
		(m[3] * v[0]) + (m[7] * v[1]) + (m[11] * v[2]) + (m[15] * v[3])
	}};
}

const char* odVertex_get_debug_string(const odVertex* vertex) {
	if (vertex == nullptr) {
		return "odVertex{this=nullptr}";
	}

	return odDebugString_format(
		"odVertex{this=%p, pos=%s, col=%s, u=%f, v=%f}",
		static_cast<const void*>(vertex),
		odVector_get_debug_string(&vertex->pos),
		odColor_get_debug_string(&vertex->col),
		static_cast<double>(vertex->u),
		static_cast<double>(vertex->v));
}
