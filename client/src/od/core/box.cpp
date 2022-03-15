#include <od/core/box.hpp>

#include <od/core/debug.h>
#include <od/core/type.h>
#include <od/core/allocation.h>

bool odBox_check_valid(const struct odBox* box) {
	if (!OD_CHECK(odAllocation_check_valid(&box->allocation))
		|| !OD_CHECK((odAllocation_get_const(&box->allocation) == nullptr) || odType_check_valid(box->type))) {
		return false;
	}

	return true;
}
const char* odBox_get_debug_string(const struct odBox* box) {
	if (box == nullptr) {
		return "null";
	}

	return odDebugString_format(
		"{\"count\": %d}",
		static_cast<int32_t>(odAllocation_get_const(&box->allocation) != nullptr));
}
void odBox_swap(struct odBox* box1, struct odBox* box2) {
	if (!OD_DEBUG_CHECK(box1 != nullptr)
		|| !OD_DEBUG_CHECK(box2 != nullptr)) {
		return;
	}


	const odType* swap_type = box1->type;
	box1->type = box2->type;
	box2->type = swap_type;

	odAllocation_swap(&box1->allocation, &box2->allocation);
}
bool odBox_init(struct odBox* box, const struct odType* type) {
	if (!OD_DEBUG_CHECK(box != nullptr)
		|| !OD_DEBUG_CHECK((type == nullptr) || odType_check_valid(type))) {
		return false;
	}

	odBox_destroy(box);

	if (type == nullptr) {
		return true;
	}

	odAllocation new_allocation{};
	if (!OD_CHECK(odAllocation_init(&new_allocation, type->size))) {
		return false;
	}

	void* new_allocation_ptr = odAllocation_get(&new_allocation);
	if (!OD_DEBUG_CHECK(new_allocation_ptr != nullptr)) {
		return false;
	}

	type->default_construct_fn(new_allocation_ptr, 1);
	odAllocation_swap(&box->allocation, &new_allocation);

	box->type = type;

	return true;
}
void odBox_init_null(struct odBox* box) {
	if (!OD_DEBUG_CHECK(box != nullptr)) {
		return;
	}

	OD_DISCARD(OD_CHECK(odBox_init(box, nullptr)));
}
void odBox_destroy(struct odBox* box) {
	if (!OD_DEBUG_CHECK(box != nullptr)) {
		return;
	}

	void* allocation_ptr = odAllocation_get(&box->allocation);
	if ((allocation_ptr != nullptr) && !OD_CHECK(odType_check_valid(box->type))) {
		box->type->destruct_fn(allocation_ptr, 1);
	}
	box->type = nullptr;

	odAllocation_destroy(&box->allocation);
}
void* odBox_get(struct odBox* box) {
	if (!OD_DEBUG_CHECK(odBox_check_valid(box))) {
		return nullptr;
	}

	return odAllocation_get(&box->allocation);
}
const void* odBox_get_const(const struct odBox* box) {
	if (!OD_DEBUG_CHECK(odBox_check_valid(box))) {
		return nullptr;
	}

	return odAllocation_get_const(&box->allocation);
}
void* odBox_ensure(struct odBox* box, const struct odType* type) {
	if (!OD_DEBUG_CHECK(odBox_check_valid(box))) {
		return nullptr;
	}

	void* elem = odBox_get(box);
	if (elem != nullptr) {
		return elem;
	}

	if (!OD_CHECK(odBox_init(box, type))) {
		return nullptr;
	}

	elem = odBox_get(box);
	if (!OD_CHECK(elem != nullptr)) {
		return nullptr;
	}

	return elem;
}

odBox::odBox()
: allocation{}, type{nullptr} {
	odBox_init_null(this);
}
odBox::odBox(const odType* in_type)
: odBox{} {
	OD_DISCARD(OD_CHECK(odBox_init(this, in_type)));
}
odBox::odBox(odBox&& other) {
	odBox_swap(this, &other);
}
odBox& odBox::operator=(odBox&& other) {
	odBox_swap(this, &other);
	return *this;
}
odBox::~odBox() {
	odBox_destroy(this);
}
