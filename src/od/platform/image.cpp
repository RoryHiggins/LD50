#include <od/platform/image.hpp>

#include <string.h>

#include <png.h>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/color.h>

const odType* odImage_get_type_constructor() {
	return odType_get<odImage>();
}
bool odImage_copy(odImage* image, const odImage* src_image) {
	if (image == nullptr) {
		OD_ERROR("image=nullptr");
		return false;
	}

	if (src_image == nullptr) {
		OD_ERROR("src_image=nullptr");
		return false;
	}

	odImage_release(image);

	uint32_t size = static_cast<uint32_t>(sizeof(odColor)) * src_image->width * src_image->height;
	if (size == 0) {
		return true;
	}

	if (!odAllocation_allocate(&image->allocation, size)) {
		return false;
	}

	void* image_data = odAllocation_get(&image->allocation);
	if (image_data == nullptr) {
		OD_ERROR("allocation failed");
		return false;
	}

	const void* src_image_data = odAllocation_get_const(&src_image->allocation);
	if (!src_image_data) {
		OD_ERROR("no source image");
		return false;
	}

	memcpy(image_data, src_image_data, size);
	image->width = src_image->width;
	image->height = src_image->height;

	return true;
}
void odImage_swap(odImage* image1, odImage* image2) {
	if (image1 == nullptr) {
		OD_ERROR("image1=nullptr");
		return;
	}

	if (image2 == nullptr) {
		OD_ERROR("image2=nullptr");
		return;
	}

	uint32_t swap_width = image1->width;
	uint32_t swap_height = image1->height;

	image1->width = image2->width;
	image1->height = image2->height;

	image2->width = swap_width;
	image2->height = swap_height;
}
const char* odImage_get_debug_string(const odImage* image) {
	if (image == nullptr) {
		return "odImage{this=nullptr}";
	}

	return odDebugString_format(
		"odImage{this=%p, allocation=%s, width=%u, height=%u}",
		static_cast<const void*>(image),
		odAllocation_get_debug_string(&image->allocation),
		image->width,
		image->height
	);
}
bool odImage_allocate(odImage* image, uint32_t width, uint32_t height) {
	if (image == nullptr) {
		OD_ERROR("image=nullptr");
		return false;
	}

	odImage_release(image);

	uint32_t size = static_cast<uint32_t>(sizeof(odColor)) * width * height;
	if (size == 0) {
		return true;
	}

	if (!odAllocation_allocate(&image->allocation, size)) {
		return false;
	}

	void* image_data = odAllocation_get(&image->allocation);
	if (image_data == nullptr) {
		OD_ERROR("allocation failed");
		return false;
	}

	image->width = width;
	image->height = height;

	return true;
}
void odImage_release(odImage* image) {
	if (image == nullptr) {
		OD_ERROR("image=nullptr");
		return;
	}

	image->height = 0;
	image->width = 0;
	odAllocation_release(&image->allocation);
}
void odImage_get_size(const odImage* image, uint32_t* out_opt_width, uint32_t* out_opt_height) {
	uint32_t unused;
	out_opt_width = (out_opt_width != nullptr) ? out_opt_width : &unused;
	out_opt_height = (out_opt_height != nullptr) ? out_opt_height : &unused;

	if (image == nullptr) {
		OD_ERROR("image=nullptr");
		return;
	}

	*out_opt_width = image->width;
	*out_opt_height = image->height;
}
bool odImage_read_png(odImage* image, const void* src_png, uint32_t src_png_size) {
	if (image == nullptr) {
		OD_ERROR("image=nullptr");
		return false;
	}

	if (src_png == nullptr) {
		OD_ERROR("src_png=nullptr");
		return false;
	}

	if (src_png_size == 0) {
		OD_ERROR("src_png_size=0");
		return false;
	}

	png_image png;
	memset(static_cast<void*>(&png), 0, sizeof(png));
	png.version = PNG_IMAGE_VERSION;

	if (png_image_begin_read_from_memory(&png, src_png, static_cast<size_t>(src_png_size)) == 0) {
		OD_ERROR("png_image_begin_read_from_memory() failed");
		return false;
	}

	png.format = PNG_FORMAT_RGBA;

	uint32_t width = png.width;
	uint32_t height = png.height;
	if (PNG_IMAGE_SIZE(png) != (width * height * sizeof(odColor))) {
		return false;
	}
	
	if (!odImage_allocate(image, width, height)) {
		return false;
	}

	void* image_data = odAllocation_get(&image->allocation);
	if (image_data == nullptr) {
		OD_ERROR("allocation failed");
		return false;
	}

	if (png_image_finish_read(&png, /*background*/ nullptr, image_data, /*row_stride*/ 0, /*colormap*/ nullptr) == 0) {
		OD_ERROR("png_image_finish_read() failed");
		return false;
	}

	image->width = width;
	image->height = height;

	return true;
}
odColor* odImage_get(odImage* image) {
	if (image == nullptr) {
		OD_ERROR("image=nullptr");
		return nullptr;
	}

	return static_cast<odColor*>(odAllocation_get(&image->allocation));
}
const odColor* odImage_get_const(const odImage* image) {
	return odImage_get(const_cast<odImage*>(image));
}

odImage::odImage()
: allocation{}, width{0}, height{0} {
}
odImage::odImage(odImage const& other)
: odImage{} {
	odImage_copy(this, &other);
}
odImage::odImage(odImage&& other)
: odImage{} {
	odImage_swap(this, &other);
}
odImage& odImage::operator=(const odImage& other) {
	odImage_copy(this, &other);
	return *this;
}
odImage& odImage::operator=(odImage&& other) {
	odImage_swap(this, &other);
	return *this;
}
odImage::~odImage() {
	odImage_release(this);
}
