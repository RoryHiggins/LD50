#include <od/core/array.hpp>

#include <od/core/debug.h>
#include <od/core/type.hpp>

#define OD_ARRAY_ERROR(ARRAY, ...) \
	OD_ERROR("%s", odArray_get_debug_string(ARRAY)); \
	OD_ERROR(__VA_ARGS__)

const odType* odArray_get_type_constructor(void) {
	return odType_get<odArray>();
}
void odArray_swap(odArray* array1, odArray* array2) {
	OD_TRACE("array1=%s, array2=%s", odArray_get_debug_string(array1), odArray_get_debug_string(array2));

	if (array1 == nullptr) {
		OD_ARRAY_ERROR(array1, "array1=nullptr");
		return;
	}

	if (array2 == nullptr) {
		OD_ARRAY_ERROR(array2, "array2=nullptr");
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
	if (array == nullptr) {
		return false;
	}

	if (array->count < 0) {
		return false;
	}

	if (array->capacity < 0) {
		return false;
	}

	if (!odType_get_valid(array->type)) {
		return false;
	}

	if (!odAllocation_get_valid(&array->allocation)) {
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
const odType* odArray_get_type(const odArray* array) {
	if (array == nullptr) {
		OD_ARRAY_ERROR(array, "array=nullptr");
		return nullptr;
	}

	return array->type;
}
void odArray_set_type(odArray* array, const odType* type) {
	OD_TRACE("array=%s, type=%s", odArray_get_debug_string(array), odType_get_debug_string(type));

	if (array == nullptr) {
		OD_ARRAY_ERROR(array, "array=nullptr");
		return;
	}

	if (!odType_get_valid(type)) {
		OD_ARRAY_ERROR(array, "type not valid, type=%s", odType_get_debug_string(type));
		return;
	}

	if (array->capacity > 0) {
		odArray_release(array);
	}

	array->type = type;
}
int32_t odArray_get_capacity(const odArray* array) {
	if (!odArray_get_valid(array)) {
		OD_ARRAY_ERROR(array, "not valid");
		return 0;
	}

	return array->capacity;
}
bool odArray_set_capacity(odArray* array, int32_t new_capacity) {
	OD_TRACE("array=%s, new_capacity=%d", odArray_get_debug_string(array), new_capacity);

	if (!odArray_get_valid(array)) {
		OD_ARRAY_ERROR(array, "not valid");
		return false;
	}

	if (new_capacity < 0) {
		OD_ARRAY_ERROR(array, "new_capacity < 0");
		return false;
	}

	int32_t new_count = (new_capacity < array->count) ? new_capacity : array->count;
	OD_TRACE("new_count=%d", new_count);

	odAllocation new_allocation;
	if (!odAllocation_allocate(&new_allocation, new_capacity * array->type->size)) {
		OD_ARRAY_ERROR(array, "new allocation failed");
		return false;
	}
	OD_TRACE("new_allocation=%s", odAllocation_get_debug_string(&new_allocation));

	void* new_allocation_ptr = odAllocation_get(&new_allocation);
	if ((new_allocation_ptr == nullptr) && (new_capacity > 0)) {
		OD_ARRAY_ERROR(array, "new_allocation_ptr=nullptr for nonzero capacity, uncaught allocation failure");
		return false;
	}

	void* old_allocation_ptr = odAllocation_get(&array->allocation);
	if ((old_allocation_ptr == nullptr) && (array->capacity > 0)) {
		OD_ARRAY_ERROR(array, "old_allocation_ptr=nullptr for nonzero capacity, likely memory leak");
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

	if (!odArray_get_valid(array)) {
		OD_ARRAY_ERROR(array, "not valid");
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
void odArray_release(odArray* array) {
	OD_TRACE("array=%s", odArray_get_debug_string(array));

	if (array == nullptr) {
		OD_ARRAY_ERROR(array, "array=nullptr");
		return;
	}

	array->count = 0;
	array->capacity = 0;
	odAllocation_release(&array->allocation);
}
int32_t odArray_get_count(const odArray* array) {
	if (!odArray_get_valid(array)) {
		OD_ARRAY_ERROR(array, "not valid");
		return 0;
	}

	return array->count;
}
bool odArray_set_count(odArray* array, int32_t new_count) {
	OD_TRACE("array=%s, new_count=%d", odArray_get_debug_string(array), new_count);

	if (!odArray_get_valid(array)) {
		OD_ARRAY_ERROR(array, "not valid");
		return false;
	}

	if (new_count < 0) {
		OD_ARRAY_ERROR(array, "new_count < 0");
		return false;
	}

	if (!odArray_ensure_capacity(array, new_count)) {
		OD_ARRAY_ERROR(array, "Failed to ensure capacity, new_count=%d", new_count);
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

	if (!odArray_get_valid(array)) {
		OD_ARRAY_ERROR(array, "not valid");
		return false;
	}

	if (out_expand_dest == nullptr) {
		OD_ARRAY_ERROR(array, "out_expand_dest=nullptr");
		return false;
	}

	if (expand_count < 0) {
		OD_ARRAY_ERROR(array, "expand_count<=0");
		return false;
	}

	*out_expand_dest = nullptr;

	if (expand_count == 0) {
		return true;
	}

	int32_t offset = array->count;
	if (!odArray_set_count(array, array->count + expand_count)) {
		OD_ARRAY_ERROR(array, "failed to expand");
		return false;
	}

	*out_expand_dest = odArray_get(array, offset);
	if (*out_expand_dest == nullptr) {
		OD_ARRAY_ERROR(array, "failed to get start of new array elements");
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

	if (!odArray_get_valid(array)) {
		OD_ARRAY_ERROR(array, "not valid");
		return false;
	}

	if (moved_src == nullptr) {
		OD_ARRAY_ERROR(array, "moved_src == nullptr");
		return false;
	}

	if (moved_count <= 0) {
		OD_ARRAY_ERROR(array, "moved_count<=0");
		return false;
	}

	void* push_dest = nullptr;
	if (!odArray_expand(array, &push_dest, moved_count)) {
		OD_ARRAY_ERROR(array, "failed to expand array");
		return false;
	}

	OD_TRACE("push_dest=%p", static_cast<const void*>(push_dest));

	array->type->move_assign_fn(push_dest, moved_src, moved_count * array->type->size);

	return true;
}
bool odArray_shrink(odArray* array, int32_t count) {
	OD_TRACE("array=%s, count=%d", odArray_get_debug_string(array), count);

	if (!odArray_get_valid(array)) {
		OD_ARRAY_ERROR(array, "not valid");
		return false;
	}

	if (count <= 0) {
		OD_ARRAY_ERROR(array, "count<=0");
		return false;
	}

	if (array->count < count) {
		OD_ARRAY_ERROR(array, "array too small, count=%d, array->count=%d", count, array->count);
		return false;
	}

	if (!odArray_set_count(array, array->count - count)) {
		OD_ARRAY_ERROR(array, "failed to set count");
		return false;
	}

	return true;
}
bool odArray_swap_pop(odArray* array, int32_t i) {
	OD_TRACE("array=%s, i=%d", odArray_get_debug_string(array), i);

	if (!odArray_get_valid(array)) {
		OD_ARRAY_ERROR(array, "not valid");
		return false;
	}

	if ((i < 0) || (i >= array->count)) {
		OD_ARRAY_ERROR(array, "outside bounds, i=%d", i);
		return false;
	}

	array->type->move_assign_fn(odArray_get(array, i), odArray_get(array, array->count - 1), array->type->size);

	return true;
}
void* odArray_get(odArray* array, int32_t i) {
	if (!odArray_get_valid(array)) {
		OD_ARRAY_ERROR(array, "not valid");
		return nullptr;
	}

	if ((i < 0) || (i >= array->count)) {
		OD_ARRAY_ERROR(array, "outside bounds, i=%d", i);
		return nullptr;
	}

	void* elements = odAllocation_get(&array->allocation);
	if (elements == nullptr) {
		OD_ARRAY_ERROR(array, "not allocated, i=%d", i);
		return nullptr;
	}

	return odType_index(array->type, elements, i);
}
const void* odArray_get_const(const odArray* array, int32_t i) {
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
