#include <od/core.h>
#include <od/core/allocation.hpp>

#include <stdlib.h>

#include <od/core/debug.hpp>
#include <od/core/type.hpp>

const odType* odAllocation_get_type_constructor(void) {
	return odType_get<odAllocation>();
}
void odAllocation_swap(odAllocation* allocation1, odAllocation* allocation2) {
	if (allocation1 == nullptr) {
		OD_ERROR("allocation1=nullptr");
		return;
	}

	if (allocation2 == nullptr) {
		OD_ERROR("allocation2=nullptr");
		return;
	}

	void* swap_ptr = allocation1->ptr;

	allocation1->ptr = allocation2->ptr;

	allocation2->ptr = swap_ptr;
}
const char* odAllocation_get_debug_string(const odAllocation* allocation) {
	if (allocation == nullptr) {
		return "odAllocation{this=nullptr}";
	}

	return odDebugString_format(
		"odAllocation{this=%p, ptr=%p}",
		static_cast<const void*>(allocation),
		static_cast<const void*>(allocation->ptr)
	);
}
bool odAllocation_allocate(odAllocation* allocation, uint32_t size) {
	if (allocation == nullptr) {
		OD_ERROR("allocation=nullptr");
		return false;
	}

	OD_TRACE("allocation=%s, size=%u", odAllocation_get_debug_string(allocation), size);

	if (size == 0) {
		OD_TRACE("size=0, ptr=%s", odAllocation_get_debug_string(allocation));
		odAllocation_release(allocation);
		return true;
	}

	void* new_allocation_ptr = realloc(allocation->ptr, size);
	if (new_allocation_ptr == nullptr) {
		OD_ERROR("allocation failed, ptr=%s, size=%u", odAllocation_get_debug_string(allocation), size);
		return false;
	}

	allocation->ptr = new_allocation_ptr;

	return true;
}
void odAllocation_release(odAllocation* allocation) {
	if (allocation == nullptr) {
		OD_ERROR("allocation=nullptr");
		return;
	}

	OD_TRACE("allocation=%s", odAllocation_get_debug_string(allocation));

	free(allocation->ptr);

	allocation->ptr = nullptr;
}
void* odAllocation_get(odAllocation* allocation) {
	if (allocation == nullptr) {
		OD_ERROR("allocation=nullptr");
		return nullptr;
	}

	return static_cast<void*>(static_cast<char*>(allocation->ptr));
}
const void* odAllocation_get_const(const odAllocation* allocation) {
	return const_cast<void*>(odAllocation_get(const_cast<odAllocation*>(allocation)));
}

odAllocation::odAllocation()
: ptr{nullptr} {
}
odAllocation::odAllocation(odAllocation&& other)
: odAllocation{} {
	odAllocation_swap(this, &other);
}
odAllocation& odAllocation::operator=(odAllocation&& other) {
	odAllocation_swap(this, &other);
	return *this;
}
odAllocation::~odAllocation() {
	odAllocation_release(this);
}
