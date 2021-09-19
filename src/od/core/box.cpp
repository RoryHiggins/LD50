#include <od/core/box.hpp>

#include <cstdlib>

#include <od/core/debug.h>
#include <od/core/type.hpp>

const odType* odBox_get_type_constructor(void) {
	return odType_get<odBox>();
}
void odBox_swap(odBox* box1, odBox* box2) {
	if (box1 == nullptr) {
		OD_ERROR("box1=nullptr");
		return;
	}

	if (box2 == nullptr) {
		OD_ERROR("box2=nullptr");
		return;
	}

	OD_TRACE("box1=%s, box2=%s", odBox_get_debug_string(box1), odBox_get_debug_string(box2));

	const odType* swap_type = box1->type;

	box1->type = box2->type;

	box2->type = swap_type;

	odAllocation_swap(&box1->allocation, &box2->allocation);
}
const char* odBox_get_debug_string(const odBox* box) {
	if (box == nullptr) {
		return "odBox{this=nullptr}";
	}

	return odDebugString_format(
		"odBox{this=%p, allocation=%s, type=%s}",
		static_cast<const void*>(box),
		odAllocation_get_debug_string(&box->allocation),
		odType_get_debug_string(box->type));
}
const odType* odBox_get_type(const odBox* box) {
	if (box == nullptr) {
		OD_ERROR("box=nullptr");
		return nullptr;
	}

	return box->type;
}
void odBox_set_type(odBox* box, const odType* type) {
	if (box == nullptr) {
		OD_ERROR("box=nullptr");
		return;
	}

	if (type == nullptr) {
		OD_ERROR("type=nullptr");
		return;
	}

	OD_TRACE("box=%s, type=%s", odBox_get_debug_string(box), odType_get_debug_string(type));

	if (box->type == type) {
		OD_TRACE("type already set, box=%s", odBox_get_debug_string(box));
		return;
	}

	if (odBox_get(box) != nullptr) {
		OD_TRACE(
			"releasing old allocation, box=%s, type=%s", odBox_get_debug_string(box), odType_get_debug_string(type));
		odBox_release(box);
	}

	box->type = type;
}
bool odBox_allocate(odBox* box) {
	if (box == nullptr) {
		OD_ERROR("box=nullptr");
		return false;
	}

	if (box->type == nullptr) {
		OD_ERROR("type not set, box=%s", odBox_get_debug_string(box));
		return false;
	}

	OD_TRACE("box=%s", odBox_get_debug_string(box));

	void* old_allocation_ptr = odAllocation_get(&box->allocation);
	if (old_allocation_ptr != nullptr) {
		OD_TRACE("releasing existing allocation, box=%s", odBox_get_debug_string(box));
		box->type->destruct_fn(old_allocation_ptr, 1);
		odBox_release(box);
	}

	if (!odAllocation_allocate(&box->allocation, box->type->size)) {
		return false;
	}

	void* new_allocation_ptr = odAllocation_get(&box->allocation);
	if (new_allocation_ptr == nullptr) {
		OD_ERROR("new_allocation_ptr=nullptr");
		return false;
	}

	box->type->default_construct_fn(new_allocation_ptr, 1);

	return true;
}
void odBox_release(odBox* box) {
	if (box == nullptr) {
		OD_ERROR("box=nullptr");
		return;
	}

	OD_TRACE("box=%s", odBox_get_debug_string(box));

	void* allocation_ptr = odAllocation_get(&box->allocation);

	if (allocation_ptr == nullptr) {
		return;
	}

	if (box->type == nullptr) {
		OD_ERROR("allocated but type not set, box=%s", odBox_get_debug_string(box));
		return;
	}

	box->type->destruct_fn(allocation_ptr, 1);

	odAllocation_release(&box->allocation);
}
void* odBox_get(odBox* box) {
	if (box == nullptr) {
		OD_ERROR("box=nullptr");
		return nullptr;
	}

	return odAllocation_get(&box->allocation);
}
const void* odBox_get_const(const odBox* box) {
	if (box == nullptr) {
		OD_ERROR("box=nullptr");
		return nullptr;
	}

	return odBox_get(const_cast<odBox*>(box));
}

odBox::odBox() : allocation{}, type{nullptr} {
}
odBox::odBox(const odType* in_type) : odBox{} {
	odBox_set_type(this, in_type);
}
odBox::odBox(odBox&& other) : odBox{} {
	odBox_swap(this, &other);
}
odBox& odBox::operator=(odBox&& other) {
	odBox_swap(this, &other);
	return *this;
}
odBox::~odBox() {
	odBox_release(this);
	type = nullptr;
}
