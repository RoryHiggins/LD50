#include <od/core/allocation.hpp>

#include <cstdlib>

#include <od/core/debug.hpp>
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
bool odAllocation_allocate(odAllocation* allocation, int32_t size) {
	OD_TRACE("allocation=%s, size=%d", odAllocation_get_debug_string(allocation), size);

	if (!OD_DEBUG_CHECK(odAllocation_get_valid(allocation))
		|| !OD_DEBUG_CHECK(size >= 0)) {
		return false;
	}

	if (size == 0) {
		odAllocation_release(allocation);
		return true;
	}

	void* new_allocation_ptr = realloc(allocation->ptr, static_cast<size_t>(size));
	if (!OD_CHECK(new_allocation_ptr != nullptr)) {
		return false;
	}
	OD_TRACE("new_allocation_ptr=%p", static_cast<const void*>(new_allocation_ptr));

	allocation->ptr = new_allocation_ptr;

	return true;
}
void odAllocation_release(odAllocation* allocation) {
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
	odAllocation_release(this);
}
