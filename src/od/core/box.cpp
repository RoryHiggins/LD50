#include <od/core.h>
#include <od/core/box.hpp>

#include <stdlib.h>

#include <od/core/type.hpp>

const odType* odBox_get_type_constructor(void) {
	return odType_get<odBox>();
}
void odBox_swap(odBox* ptr1, odBox* ptr2) {
	if (ptr1 == nullptr) {
		OD_ERROR("ptr1=nullptr");
		return;
	}

	if (ptr2 == nullptr) {
		OD_ERROR("ptr2=nullptr");
		return;
	}

	OD_TRACE("ptr1=%s, ptr2=%s", odBox_get_debug_string(ptr1), odBox_get_debug_string(ptr2));

	const odType* swap_type = ptr1->type;

	ptr1->type = ptr2->type;

	ptr2->type = swap_type;

	odAllocation_swap(&ptr1->allocation, &ptr2->allocation);
}
const char* odBox_get_debug_string(const odBox* ptr) {
	if (ptr == nullptr) {
		return "odBox{this=nullptr}";
	}

	return odDebugString_create_formatted(
		"odBox{this=%p, allocation=%s, type=%s}",
		static_cast<const void*>(ptr),
		odAllocation_get_debug_string(&ptr->allocation),
		odType_get_debug_string(ptr->type)
	);
}
const odType* odBox_get_type(const odBox* ptr) {
	if (ptr == nullptr) {
		OD_ERROR("ptr=nullptr");
		return nullptr;
	}

	return ptr->type;
}
void odBox_set_type(odBox* ptr, const odType* type) {
	if (ptr == nullptr) {
		OD_ERROR("ptr=nullptr");
		return;
	}

	if (type == nullptr) {
		OD_ERROR("type=nullptr");
		return;
	}

	OD_TRACE("ptr=%s, type=%s", odBox_get_debug_string(ptr), odType_get_debug_string(type));

	if (ptr->type == type) {
		OD_TRACE("type already set, ptr=%s", odBox_get_debug_string(ptr));
		return;
	}

	if (odBox_get(ptr) != nullptr) {
		OD_TRACE(
			"releasing old allocation, ptr=%s, type=%s",
			odBox_get_debug_string(ptr),
			odType_get_debug_string(type)
		);
		odBox_release(ptr);
	}

	ptr->type = type;
}
bool odBox_allocate(odBox* ptr) {
	if (ptr == nullptr) {
		OD_ERROR("ptr=nullptr");
		return false;
	}

	if (ptr->type == nullptr) {
		OD_ERROR("type not set, ptr=%s", odBox_get_debug_string(ptr));
		return false;
	}

	OD_TRACE("ptr=%s", odBox_get_debug_string(ptr));

	void* old_allocation_ptr = odAllocation_get(&ptr->allocation);
	if (old_allocation_ptr != nullptr) {
		OD_TRACE("releasing existing allocation, ptr=%s", odBox_get_debug_string(ptr));
		ptr->type->destruct_fn(old_allocation_ptr, 1);
		odBox_release(ptr);
	}

	if (!odAllocation_allocate(&ptr->allocation, ptr->type->size)) {
		return false;
	}

	void* new_allocation_ptr = odAllocation_get(&ptr->allocation);
	if (new_allocation_ptr == nullptr) {
		OD_ERROR("new_allocation_ptr=nullptr");
		return false;
	}

	ptr->type->default_construct_fn(new_allocation_ptr, 1);

	return true;
}
void odBox_release(odBox* ptr) {
	if (ptr == nullptr) {
		OD_ERROR("ptr=nullptr");
		return;
	}

	OD_TRACE("ptr=%s", odBox_get_debug_string(ptr));

	void* allocation_ptr = odAllocation_get(&ptr->allocation);

	if (allocation_ptr == nullptr) {
		return;
	}

	if (ptr->type == nullptr) {
		OD_ERROR("allocated but type not set, ptr=%s", odBox_get_debug_string(ptr));
		return;
	}

	ptr->type->destruct_fn(allocation_ptr, 1);

	odAllocation_release(&ptr->allocation);
}
void* odBox_get(odBox* ptr) {
	if (ptr == nullptr) {
		OD_ERROR("ptr=nullptr");
		return nullptr;
	}

	return odAllocation_get(&ptr->allocation);
}
const void* odBox_get_const(const odBox* ptr) {
	return const_cast<void*>(odBox_get(const_cast<odBox*>(ptr)));
}

odBox::odBox()
 : allocation{}, type{nullptr} {
}
odBox::odBox(const odType* in_type)
: odBox{} {
	odBox_set_type(this, in_type);
}
odBox::odBox(odBox&& other)
: odBox{} {
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
