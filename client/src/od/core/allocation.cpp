#include <od/core/allocation.hpp>

#include <cstdlib>

#include <od/core/debug.h>
#include <od/core/type.hpp>

const odType* odAllocation_get_type_constructor(void) {
	return odType_get<odAllocation>();
}
void odAllocation_swap(odAllocation* allocation1, odAllocation* allocation2) {
	if (!OD_DEBUG_CHECK(odAllocation_get_valid(allocation1))
		|| !OD_DEBUG_CHECK(odAllocation_get_valid(allocation1))) {
		return;
	}

	void* swap_ptr = allocation1->ptr;

	allocation1->ptr = allocation2->ptr;

	allocation2->ptr = swap_ptr;
}
bool odAllocation_get_valid(const odAllocation* allocation) {
	if (allocation == nullptr) {
		return false;
	}

	return true;
}
const char* odAllocation_get_debug_string(const odAllocation* allocation) {
	if (allocation == nullptr) {
		return "odAllocation{this=nullptr}";
	}

	return odDebugString_format(
		"odAllocation{this=%p, ptr=%p}",
		static_cast<const void*>(allocation),
		static_cast<const void*>(allocation->ptr));
}
bool odAllocation_init(odAllocation* allocation, int32_t allocation_size) {
	OD_TRACE("allocation=%s, allocation_size=%d", odAllocation_get_debug_string(allocation), allocation_size);

	if (!OD_DEBUG_CHECK(odAllocation_get_valid(allocation))
		|| !OD_DEBUG_CHECK(allocation_size >= 0)) {
		return false;
	}

	odAllocation_destroy(allocation);

	if (allocation_size == 0) {
		return true;
	}

	allocation->ptr = calloc(1, static_cast<size_t>(allocation_size));
	if (!OD_CHECK(allocation->ptr != nullptr)) {
		return false;
	}
	OD_TRACE("allocation->ptr=%p", static_cast<const void*>(allocation->ptr));

	return true;
}
void odAllocation_destroy(odAllocation* allocation) {
	OD_TRACE("allocation=%s", odAllocation_get_debug_string(allocation));

	if (!OD_DEBUG_CHECK(odAllocation_get_valid(allocation))) {
		return;
	}

	free(allocation->ptr);

	allocation->ptr = nullptr;
}
void* odAllocation_get(odAllocation* allocation) {
	if (!OD_DEBUG_CHECK(odAllocation_get_valid(allocation))) {
		return nullptr;
	}

	return static_cast<void*>(static_cast<char*>(allocation->ptr));
}
const void* odAllocation_get_const(const odAllocation* allocation) {
	return odAllocation_get(const_cast<odAllocation*>(allocation));
}
odAllocation::odAllocation() : ptr{nullptr} {
}
odAllocation::odAllocation(odAllocation&& other) : odAllocation{} {
	odAllocation_swap(this, &other);
}
odAllocation& odAllocation::operator=(odAllocation&& other) {
	odAllocation_swap(this, &other);
	return *this;
}
odAllocation::~odAllocation() {
	odAllocation_destroy(this);
}
