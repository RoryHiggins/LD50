#include <od/core/array.hpp>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/allocation.hpp>

const odType* odArray_get_type_constructor() {
	return odType_get<odArray>();
}
void odArray_swap(odArray* array1, odArray* array2) {
	OD_TRACE("array1=%s, array2=%s", odArray_get_debug_string(array1), odArray_get_debug_string(array2));

	if (!OD_DEBUG_CHECK(array1 != nullptr)
		|| !OD_DEBUG_CHECK(array2 != nullptr)) {
		return;
	}

	const odType* swap_type = array1->type;
	int32_t swap_count = array1->count;
	int32_t swap_capacity = array1->count;

	array1->type = array2->type;
	array1->count = array2->count;
	array1->capacity = array2->capacity;

	array2->type = swap_type;
	array2->count = swap_count;
	array2->capacity = swap_capacity;

	odAllocation_swap(&array1->allocation, &array2->allocation);
}
bool odArray_get_valid(const odArray* array) {
	if ((array == nullptr)
		|| (array->count < 0)
		|| (array->capacity < 0)
		|| (!odType_get_valid(array->type))
		|| (!odAllocation_get_valid(&array->allocation))) {
		return false;
	}

	return true;
}
const char* odArray_get_debug_string(const odArray* array) {
	if (array == nullptr) {
		return "odArray{this=nullptr}";
	}

	return odDebugString_format(
		"odArray{this=%p, type=%s, allocation=%s, capacity=%d, count=%d}",
		static_cast<const void*>(array),
		odType_get_debug_string(array->type),
		odAllocation_get_debug_string(&array->allocation),
		array->capacity,
		array->count);
}
bool odArray_init(odArray* array, const odType* type) {
	OD_TRACE("array=%s, type=%s", odArray_get_debug_string(array), odType_get_debug_string(type));

	if (!OD_DEBUG_CHECK(array != nullptr)
		|| !OD_DEBUG_CHECK(odType_get_valid(type))) {
		return false;
	}

	odArray_destroy(array);

	array->type = type;
	return true;
}
void odArray_destroy(odArray* array) {
	OD_TRACE("array=%s", odArray_get_debug_string(array));

	if (!OD_DEBUG_CHECK(array != nullptr)) {
		return;
	}

	array->count = 0;
	array->capacity = 0;
	odAllocation_destroy(&array->allocation);
}
const odType* odArray_get_type(const odArray* array) {
	if (!OD_DEBUG_CHECK(array != nullptr)) {
		return nullptr;
	}

	return array->type;
}
int32_t odArray_get_capacity(const odArray* array) {
	if (!OD_DEBUG_CHECK(odArray_get_valid(array))) {
		return 0;
	}

	return array->capacity;
}
bool odArray_set_capacity(odArray* array, int32_t new_capacity) {
	OD_TRACE("array=%s, new_capacity=%d", odArray_get_debug_string(array), new_capacity);

	if (!OD_DEBUG_CHECK(odArray_get_valid(array))
		|| !OD_DEBUG_CHECK(new_capacity >= 0)) {
		return false;
	}

	int32_t new_count = (new_capacity < array->count) ? new_capacity : array->count;
	OD_TRACE("new_count=%d", new_count);

	odAllocation new_allocation;
	if (!OD_CHECK(odAllocation_init(&new_allocation, new_capacity * array->type->size))) {
		return false;
	}
	OD_TRACE("new_allocation=%s", odAllocation_get_debug_string(&new_allocation));

	void* new_allocation_ptr = odAllocation_get(&new_allocation);
	if (!OD_DEBUG_CHECK((new_allocation_ptr != nullptr) || (new_capacity == 0))) {
		return false;
	}

	void* old_allocation_ptr = odAllocation_get(&array->allocation);
	if (!OD_DEBUG_CHECK((old_allocation_ptr != nullptr) || (array->capacity == 0))) {
		return false;
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
bool odArray_ensure_capacity(odArray* array, int32_t min_capacity) {
	OD_TRACE("array=%s, min_capacity=%d", odArray_get_debug_string(array), min_capacity);

	if (!OD_DEBUG_CHECK(odArray_get_valid(array))) {
		return false;
	}

	int32_t capacity = odArray_get_capacity(array);
	if (capacity >= min_capacity) {
		return true;
	}

	int32_t new_capacity = 0;

	const int32_t start_capacity = 16;
	if (min_capacity <= start_capacity) {
		new_capacity = start_capacity;
	} else if ((capacity * 2) >= min_capacity) {
		new_capacity = capacity * 2;
	} else {
		new_capacity = min_capacity;
	}

	return odArray_set_capacity(array, new_capacity);
}
int32_t odArray_get_count(const odArray* array) {
	if (!OD_DEBUG_CHECK(odArray_get_valid(array))) {
		return 0;
	}

	return array->count;
}
bool odArray_set_count(odArray* array, int32_t new_count) {
	OD_TRACE("array=%s, new_count=%d", odArray_get_debug_string(array), new_count);

	if (!OD_DEBUG_CHECK(odArray_get_valid(array))
		|| !OD_DEBUG_CHECK(new_count >= 0)) {
		return false;
	}

	if (!OD_CHECK(odArray_ensure_capacity(array, new_count))) {
		return false;
	}

	if (new_count < array->count) {
		int32_t default_count = array->count - new_count;
		void* popped_elements = odArray_get(array, new_count);
		array->type->destruct_fn(popped_elements, default_count);
		array->type->default_construct_fn(popped_elements, default_count);
	}

	array->count = new_count;

	return true;
}
bool odArray_expand(odArray* array, void** out_expand_dest, int32_t expand_count) {
	OD_TRACE(
		"array=%s, out_expand_dest=%p, expand_count=%d",
		odArray_get_debug_string(array),
		static_cast<const void*>(out_expand_dest),
		expand_count);

	if (!OD_DEBUG_CHECK(odArray_get_valid(array))
		|| !OD_DEBUG_CHECK(out_expand_dest != nullptr)
		|| !OD_DEBUG_CHECK(expand_count >= 0)) {
		return false;
	}

	*out_expand_dest = nullptr;

	if (expand_count == 0) {
		return true;
	}

	int32_t offset = array->count;
	if (!OD_CHECK(odArray_set_count(array, array->count + expand_count))) {
		return false;
	}

	*out_expand_dest = odArray_get(array, offset);
	if (!OD_CHECK(*out_expand_dest != nullptr)) {
		return false;
	}

	return true;
}
bool odArray_push(odArray* array, void* moved_src, int32_t moved_count) {
	OD_TRACE(
		"array=%s, moved_src=%p, moved_count=%d",
		odArray_get_debug_string(array),
		static_cast<const void*>(moved_src),
		moved_count);

	if (!OD_DEBUG_CHECK(odArray_get_valid(array))
		|| !OD_DEBUG_CHECK(moved_src != nullptr)
		|| !OD_DEBUG_CHECK(moved_count > 0)) {
		return false;
	}

	void* push_dest = nullptr;
	if (!OD_CHECK(odArray_expand(array, &push_dest, moved_count))) {
		return false;
	}

	OD_TRACE("push_dest=%p", static_cast<const void*>(push_dest));

	array->type->move_assign_fn(push_dest, moved_src, moved_count * array->type->size);

	return true;
}
bool odArray_pop(odArray* array, int32_t count) {
	OD_TRACE("array=%s, count=%d", odArray_get_debug_string(array), count);

	if (!OD_DEBUG_CHECK(odArray_get_valid(array))
		|| !OD_DEBUG_CHECK(count > 0)
		|| !OD_DEBUG_CHECK(array->count >= count)) {
		return false;
	}

	if (!OD_CHECK(odArray_set_count(array, array->count - count))) {
		return false;
	}

	return true;
}
bool odArray_swap_pop(odArray* array, int32_t i) {
	OD_TRACE("array=%s, i=%d", odArray_get_debug_string(array), i);

	if (!OD_DEBUG_CHECK(odArray_get_valid(array))
		|| !OD_DEBUG_CHECK(i >= 0)
		|| !OD_CHECK(i < array->count)) {
		return false;
	}

	array->type->move_assign_fn(odArray_get(array, i), odArray_get(array, array->count - 1), array->type->size);

	return true;
}
void* odArray_get(odArray* array, int32_t i) {
	if (!OD_DEBUG_CHECK(odArray_get_valid(array))
		|| !OD_DEBUG_CHECK(i >= 0)
		|| !OD_CHECK(i < array->count)) {
		return nullptr;
	}

	void* elements = odAllocation_get(&array->allocation);
	if (!OD_CHECK(elements != nullptr)) {
		return nullptr;
	}

	void* result = odType_index(array->type, elements, i);
	if (!OD_DEBUG_CHECK(result != nullptr)) {
		return nullptr;
	}
	return result;
}
const void* odArray_get_const(const odArray* array, int32_t i) {
	return odArray_get(const_cast<odArray*>(array), i);
}
void* odArray_begin(odArray* array) {
	if (!OD_DEBUG_CHECK(array != nullptr)) {
		return nullptr;
	}

	return array->allocation.ptr;
}
const void* odArray_begin_const(const odArray* array) {
	return odArray_begin(const_cast<odArray*>(array));
}
void* odArray_end(odArray* array) {
	if (!OD_DEBUG_CHECK(array != nullptr)
		|| !OD_DEBUG_CHECK(array->count >= 0)) {
		return nullptr;
	}

	void* result = odType_index(array->type, array->allocation.ptr, array->count);
	if (!OD_DEBUG_CHECK(result != nullptr)) {
		return nullptr;
	}
	return result;
}
const void* odArray_end_const(const odArray* array) {
	return odArray_end(const_cast<odArray*>(array));
}
odArray::odArray() : allocation{}, type{nullptr}, capacity{0}, count{0} {
}
odArray::odArray(const odType* in_type) : odArray{} {
	OD_ASSERT(odArray_init(this, in_type));
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

	odArray_destroy(this);

	type = nullptr;
}
