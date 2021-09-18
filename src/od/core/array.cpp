#include <od/core/array.hpp>

#include <od/core/debug.h>
#include <od/core/type.hpp>

const odType* odArray_get_type_constructor(void) {
	return odType_get<odArray>();
}
void odArray_swap(odArray* array1, odArray* array2) {
	if (array1 == nullptr) {
		OD_ERROR("array1=nullptr");
		return;
	}

	if (array2 == nullptr) {
		OD_ERROR("array2=nullptr");
		return;
	}

	const odType* swap_type = array1->type;
	uint32_t swap_count = array1->count;
	uint32_t swap_capacity = array1->count;

	array1->type = array2->type;
	array1->count = array2->count;
	array1->capacity = array2->capacity;

	array2->type = swap_type;
	array2->count = swap_count;
	array2->capacity = swap_capacity;

	odAllocation_swap(&array1->allocation, &array2->allocation);
}
const char* odArray_get_debug_string(const odArray* array) {
	if (array == nullptr) {
		return "odArray{this=nullptr}";
	}

	return odDebugString_format(
		"odArray{this=%p, type=%s, allocation=%s, capacity=%u, count=%u}",
		static_cast<const void*>(array),
		odType_get_debug_string(array->type),
		odAllocation_get_debug_string(&array->allocation),
		array->capacity,
		array->count);
}
const odType* odArray_get_type(const odArray* array) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return nullptr;
	}

	return array->type;
}
void odArray_set_type(odArray* array, const odType* type) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return;
	}

	if (type == nullptr) {
		OD_ERROR("type=nullptr");
		return;
	}

	OD_TRACE("array=%s, type=%s", odArray_get_debug_string(array), odType_get_debug_string(type));

	if (array->capacity > 0) {
		OD_TRACE(
			"releasing old allocation, array=%s, type=%s",
			odArray_get_debug_string(array),
			odType_get_debug_string(type));
		odArray_release(array);
	}

	array->type = type;
}
uint32_t odArray_get_capacity(const odArray* array) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return 0;
	}

	return array->capacity;
}
bool odArray_set_capacity(odArray* array, uint32_t new_capacity) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return false;
	}

	if (array->type == nullptr) {
		OD_ERROR("No type associated with array, cannot allocate");
		return false;
	}

	uint32_t new_count = (new_capacity < array->count) ? new_capacity : array->count;

	OD_TRACE("array=%s, new_capacity=%u, new_count=%u", odArray_get_debug_string(array), new_capacity, new_count);

	odAllocation new_allocation;
	if (!odAllocation_allocate(&new_allocation, new_capacity * array->type->size)) {
		return false;
	}

	void* old_allocation_ptr = odAllocation_get(&array->allocation);
	if ((old_allocation_ptr == nullptr) && (array->capacity > 0)) {
		OD_ERROR("old_allocation_ptr=nullptr for nonzero capacity");
	}

	void* new_allocation_ptr = odAllocation_get(&new_allocation);
	if ((new_allocation_ptr == nullptr) && (new_capacity > 0)) {
		OD_ERROR("new_allocation_ptr=nullptr for nonzero capacity");
	}

	if ((new_allocation_ptr != nullptr) && (new_capacity > 0)) {
		array->type->default_construct_fn(new_allocation_ptr, new_capacity);
	}

	if ((old_allocation_ptr != nullptr) && (new_allocation_ptr != nullptr) && (new_count > 0)) {
		array->type->move_assign_fn(new_allocation_ptr, old_allocation_ptr, new_count);
	}

	if ((old_allocation_ptr != nullptr) && (array->capacity > 0)) {
		array->type->destruct_fn(old_allocation_ptr, array->capacity);
	}

	odAllocation_swap(&array->allocation, &new_allocation);
	array->capacity = new_capacity;
	array->count = new_count;

	return true;
}
bool odArray_ensure_capacity(odArray* array, uint32_t min_capacity) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return false;
	}

	OD_TRACE("array=%s, min_capacity=%u", odArray_get_debug_string(array), min_capacity);

	uint32_t capacity = odArray_get_capacity(array);
	if (capacity >= min_capacity) {
		return true;
	}

	// over-allocate by 1 to make space for sentinel values / null terminators
	uint32_t new_capacity = min_capacity + 1;

	if ((capacity * 2) >= min_capacity) {
		new_capacity = capacity * 2;
	}

	const uint32_t start_capacity = min_capacity;
	if (min_capacity <= start_capacity) {
		new_capacity = start_capacity;
	}

	return odArray_set_capacity(array, new_capacity);
}
void odArray_release(odArray* array) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return;
	}

	OD_TRACE("array=%s", odArray_get_debug_string(array));

	array->count = 0;
	array->capacity = 0;
	odAllocation_release(&array->allocation);
}
uint32_t odArray_get_count(const odArray* array) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return 0;
	}

	return array->count;
}
bool odArray_set_count(odArray* array, uint32_t new_count) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return false;
	}

	if (array->type == nullptr) {
		OD_ERROR("No type associated with array");
		return false;
	}

	if (!odArray_ensure_capacity(array, new_count)) {
		OD_ERROR("Failed to ensure capacity, new_count=%u", new_count);
		return false;
	}

	OD_TRACE("array=%s, new_count=%u", odArray_get_debug_string(array), new_count);

	if (new_count < array->count) {
		uint32_t default_count = array->count - new_count;
		void* popped_elements = odArray_get(array, new_count);
		array->type->destruct_fn(popped_elements, default_count);
		array->type->default_construct_fn(popped_elements, default_count);
	}

	array->count = new_count;

	return true;
}
bool odArray_expand(odArray* array, void** out_expand_dest, uint32_t expand_count) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return false;
	}

	if (out_expand_dest == nullptr) {
		OD_ERROR("out_expand_dest=nullptr");
		return false;
	}

	if (expand_count == 0) {
		OD_ERROR("expand_count=0");
		return false;
	}

	if (array->type == nullptr) {
		OD_ERROR("No type associated with array");
		return false;
	}

	OD_TRACE(
		"array=%s, out_expand_dest=%p, expand_count=%u",
		odArray_get_debug_string(array),
		static_cast<const void*>(out_expand_dest),
		expand_count);

	*out_expand_dest = nullptr;

	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return false;
	}

	uint32_t start_count = array->count;
	if (!odArray_set_count(array, start_count + expand_count)) {
		return false;
	}

	*out_expand_dest = odArray_get(array, start_count);
	if (*out_expand_dest == nullptr) {
		return false;
	}

	return true;
}
bool odArray_push(odArray* array, void* moved_src, uint32_t moved_count) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return false;
	}

	if ((moved_src == nullptr) && (moved_count > 0)) {
		OD_ERROR("moved_src == nullptr");
		return false;
	}

	if (moved_count == 0) {
		OD_ERROR("moved_count=0");
		return false;
	}

	if (array->type == nullptr) {
		OD_ERROR("No type associated with array");
		return false;
	}

	OD_TRACE(
		"array=%s, moved_src=%p, moved_count=%u",
		odArray_get_debug_string(array),
		static_cast<const void*>(moved_src),
		moved_count);

	void* push_dest = nullptr;
	if (!odArray_expand(array, &push_dest, moved_count)) {
		return false;
	}

	array->type->move_assign_fn(push_dest, moved_src, moved_count * array->type->size);

	return true;
}
bool odArray_pop(odArray* array, uint32_t count) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return false;
	}

	if (array->count < count) {
		OD_ERROR("array too small, count=%u, array->count=%u", count, array->count);
		return false;
	}

	if (!odArray_set_count(array, array->count - count)) {
		return false;
	}

	OD_TRACE("array=%s, count=%u", odArray_get_debug_string(array), count);

	return true;
}
bool odArray_swap_pop(odArray* array, uint32_t i) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return false;
	}

	OD_TRACE("array=%s, i=%u", odArray_get_debug_string(array), i);

	if (array->type == nullptr) {
		OD_ERROR("No type associated with array");
		return false;
	}

	if (i >= array->count) {
		OD_ERROR("outside bounds, i=%u", i);
		return false;
	}

	array->type->move_assign_fn(odArray_get(array, i), odArray_get(array, array->count - 1), array->type->size);

	return true;
}
void* odArray_get(odArray* array, uint32_t i) {
	if (array == nullptr) {
		OD_ERROR("array=nullptr");
		return nullptr;
	}

	void* elements = odAllocation_get(&array->allocation);
	if (elements == nullptr) {
		OD_ERROR("not allocated, ptr=%s, i=%u", odArray_get_debug_string(array), i);
		return nullptr;
	}

	if (i >= array->count) {
		OD_ERROR("out of bounds, array=%s, i=%u", odArray_get_debug_string(array), i);
		return nullptr;
	}

	return odType_index(array->type, elements, i);
}
const void* odArray_get_const(const odArray* array, uint32_t i) {
	return odArray_get(const_cast<odArray*>(array), i);
}

odArray::odArray() : allocation{}, type{nullptr}, capacity{0}, count{0} {
}
odArray::odArray(const odType* in_type) : odArray{} {
	odArray_set_type(this, in_type);
}
odArray::odArray(odArray&& other) : odArray{} {
	odArray_swap(this, &other);
}
odArray& odArray::operator=(odArray&& other) {
	odArray_swap(this, &other);
	return *this;
}
odArray::~odArray() {
	OD_TRACE("array=%s", odArray_get_debug_string(this));

	odArray_release(this);

	type = nullptr;
}
