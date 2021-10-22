#include <od/core/transform.h>

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
