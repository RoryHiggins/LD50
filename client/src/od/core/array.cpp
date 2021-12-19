#include <od/core/array.hpp>

#include <cstdio>
#include <cstring>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/allocation.hpp>

const struct odType* odTrivialArray_get_type_constructor() {
	return odType_get<odTrivialArray>();
}
void odTrivialArray_swap(odTrivialArray* array1, odTrivialArray* array2) {
	if (!OD_DEBUG_CHECK(array1 != nullptr)
		|| !OD_DEBUG_CHECK(array2 != nullptr)) {
		return;
	}

	int32_t swap_count = array1->count;
	int32_t swap_capacity = array1->count;

	array1->count = array2->count;
	array1->capacity = array2->capacity;

	array2->count = swap_count;
	array2->capacity = swap_capacity;

	odAllocation_swap(&array1->allocation, &array2->allocation);
}
bool odTrivialArray_check_valid(const odTrivialArray* array) {
	if (!OD_CHECK(array != nullptr)
		|| !OD_CHECK(array->count >= 0)
		|| !OD_CHECK(array->capacity >= 0)
		|| !OD_CHECK(odAllocation_check_valid(&array->allocation))) {
		return false;
	}

	return true;
}
const char* odTrivialArray_get_debug_string(const odTrivialArray* array) {
	if (array == nullptr) {
		return "null";
	}

	return odDebugString_format("{\"count\": %d}",array->count);
}
bool odTrivialArray_init(odTrivialArray* array) {
	if (!OD_DEBUG_CHECK(array != nullptr)) {
		return false;
	}

	odTrivialArray_destroy(array);

	return true;
}
void odTrivialArray_destroy(odTrivialArray* array) {
	if (!OD_DEBUG_CHECK(array != nullptr)) {
		return;
	}

	array->count = 0;
	array->capacity = 0;
	odAllocation_destroy(&array->allocation);
}
int32_t odTrivialArray_compare(const odTrivialArray* array1, const odTrivialArray* array2, int32_t stride) {
	if (!OD_DEBUG_CHECK(array1 != nullptr)
		|| !OD_DEBUG_CHECK(array2 != nullptr)
		|| !OD_DEBUG_CHECK(stride > 0)) {
		return 0;
	}

	const void* bytes1_ptr = odTrivialArray_begin_const(array1);
	const void* bytes2_ptr = odTrivialArray_begin_const(array2);

	bytes1_ptr = bytes1_ptr ? bytes1_ptr : "";
	bytes2_ptr = bytes2_ptr ? bytes2_ptr : "";

	int32_t bytes1_count = odTrivialArray_get_count(array1) * stride;
	int32_t bytes2_count = odTrivialArray_get_count(array2) * stride;

	if (!OD_CHECK((bytes1_count == 0) || ((bytes1_ptr != nullptr)))
		|| !OD_CHECK((bytes2_count == 0) || ((bytes2_ptr != nullptr)))) {
		return false;
	}

	const int32_t ordering_equal = 0;
	const int32_t ordering_lhs_first = -1;
	const int32_t ordering_rhs_first = 1;

	int32_t min_count = (bytes1_count <= bytes2_count) ? bytes1_count : bytes2_count;
	int32_t order = ordering_equal;
	if (min_count > 0) {
		order = static_cast<int32_t>(memcmp(bytes1_ptr, bytes2_ptr, static_cast<size_t>(min_count)));
	}

	if ((order == ordering_equal) && (bytes1_count != bytes2_count)) {
		order = (bytes1_count <= bytes2_count) ? ordering_lhs_first : ordering_rhs_first;
	}

	return order;
}
int32_t odTrivialArray_get_capacity(const odTrivialArray* array) {
	if (!OD_DEBUG_CHECK(odTrivialArray_check_valid(array))) {
		return 0;
	}

	return array->capacity;
}
bool odTrivialArray_set_capacity(odTrivialArray* array, int32_t new_capacity, int32_t stride) {
	if (!OD_DEBUG_CHECK(odTrivialArray_check_valid(array))
		|| !OD_DEBUG_CHECK(new_capacity >= 0)
		|| !OD_DEBUG_CHECK(stride > 0)) {
		return false;
	}

	if (array->capacity == new_capacity) {
		return true;
	}

	int32_t new_size = new_capacity * stride;

	odAllocation new_allocation;
	// over-allocate to guarantee null termination of allocated strings
	int32_t new_allocation_size = new_size + static_cast<int32_t>(sizeof(char32_t));
	if (!OD_CHECK(odAllocation_init(&new_allocation, new_allocation_size))) {
		return false;
	}

	int32_t old_size = array->count * static_cast<int32_t>(stride);
	int32_t moved_size = ((new_size < old_size) ? new_size : old_size);
	if ((moved_size > 0) && OD_DEBUG_CHECK(array->allocation.ptr != nullptr)) {
		memcpy(new_allocation.ptr, array->allocation.ptr, static_cast<size_t>(moved_size));
	}

	odAllocation_swap(&array->allocation, &new_allocation);
	array->capacity = new_capacity;
	array->count = (new_capacity < array->count) ? new_capacity : array->count;

	return true;
}
static int32_t odTrivialArray_get_new_capacity(int32_t capacity, int32_t min_capacity) {
	if (capacity >= min_capacity) {
		return capacity;
	}

	int32_t new_capacity = min_capacity;

	const int32_t start_capacity = 16;
	if (min_capacity == 0) {
		new_capacity = 0;
	} else if (min_capacity <= start_capacity) {
		new_capacity = start_capacity;
	} else if (((capacity / 8) + 1) >= min_capacity) {
		new_capacity = (capacity / 8) + 1;  // 2x grow rate so interleaved push+pop can't always reallocate
	} else if ((capacity * 4) >= min_capacity) {
		new_capacity = capacity * 4;
	}

	return new_capacity;
}
bool odTrivialArray_ensure_capacity(odTrivialArray* array, int32_t min_capacity, int32_t stride) {
	if (!OD_DEBUG_CHECK(odTrivialArray_check_valid(array))
		|| !OD_DEBUG_CHECK(stride > 0)) {
		return false;
	}

	int32_t new_capacity = odTrivialArray_get_new_capacity(array->capacity, min_capacity);
	if (array->capacity == new_capacity) {
		return true;
	}

	return odTrivialArray_set_capacity(array, new_capacity, stride);
}
int32_t odTrivialArray_get_count(const odTrivialArray* array) {
	if (!OD_DEBUG_CHECK(odTrivialArray_check_valid(array))) {
		return 0;
	}

	return array->count;
}
bool odTrivialArray_set_count(odTrivialArray* array, int32_t new_count, int32_t stride) {
	if (!OD_DEBUG_CHECK(odTrivialArray_check_valid(array))
		|| !OD_DEBUG_CHECK(new_count >= 0)
		|| !OD_DEBUG_CHECK(stride > 0)) {
		return false;
	}

	if (!OD_CHECK(odTrivialArray_ensure_capacity(array, new_count, stride))) {
		return false;
	}

	if (new_count < array->count) {
		int32_t popped_count = array->count - new_count;
		void* popped_elements = odTrivialArray_get(array, new_count, stride);
		memset(popped_elements, 0, static_cast<size_t>(popped_count * stride));
	}

	array->count = new_count;

	return true;
}
bool odTrivialArray_extend(odTrivialArray* array, const void* extend_src, int32_t extend_count, int32_t stride) {
	if (!OD_DEBUG_CHECK(odTrivialArray_check_valid(array))
		|| !OD_DEBUG_CHECK((extend_count == 0) || (extend_src != nullptr))
		|| !OD_DEBUG_CHECK(extend_count >= 0)
		|| !OD_DEBUG_CHECK(stride > 0)) {
		return false;
	}

	if (extend_count == 0) {
		return true;
	}

	int32_t old_count = array->count;
	if (!OD_CHECK(odTrivialArray_set_count(array, old_count + extend_count, stride))) {
		return false;
	}

	void* extend_dest = odTrivialArray_get(array, old_count, stride);
	if (!OD_CHECK(extend_dest != nullptr)) {
		return false;
	}

	memcpy(extend_dest, extend_src, static_cast<size_t>(extend_count * stride));
	return true;
}
bool odTrivialArray_pop(odTrivialArray* array, int32_t pop_count, int32_t stride) {
	if (!OD_DEBUG_CHECK(pop_count >= 0)) {
		return false;
	}

	return odTrivialArray_set_count(array, array->count - pop_count, stride);
}
bool odTrivialArray_swap_pop(odTrivialArray* array, int32_t i, int32_t stride) {
	if (!OD_DEBUG_CHECK(odTrivialArray_check_valid(array))
		|| !OD_DEBUG_CHECK((i >= 0) && (i < array->count))
		|| !OD_DEBUG_CHECK(stride > 0)) {
		return false;
	}

	void* elem = odTrivialArray_get(array, i, stride);
	void* last_elem = odTrivialArray_get(array, array->count - 1, stride);

	if (!OD_DEBUG_CHECK(elem != nullptr)
		|| !OD_DEBUG_CHECK(last_elem != nullptr)) {
		return false;
	}

	memmove(elem, last_elem, static_cast<size_t>(1 * stride));

	return odTrivialArray_pop(array, 1, stride);
}
bool odTrivialArray_assign(odTrivialArray* array, const void* assign_src, int32_t assign_count, int32_t stride) {
	if (!OD_DEBUG_CHECK(odTrivialArray_check_valid(array))
		|| !OD_DEBUG_CHECK((assign_count == 0) || (assign_src != nullptr))
		|| !OD_DEBUG_CHECK(assign_count >= 0)
		|| !OD_DEBUG_CHECK(stride > 0)) {
		return false;
	}

	if (!OD_CHECK(odTrivialArray_set_count(array, 0, stride))) {
		return false;
	}

	return odTrivialArray_extend(array, assign_src, assign_count, stride);
}
void* odTrivialArray_get(odTrivialArray* array, int32_t i, int32_t stride) {
	if (!OD_DEBUG_CHECK(odTrivialArray_check_valid(array))
		|| !OD_DEBUG_CHECK((i >= 0) && (i < array->count))
		|| !OD_DEBUG_CHECK(stride > 0)
		|| !OD_DEBUG_CHECK(array->allocation.ptr != nullptr)) {
		return nullptr;
	}

	return static_cast<void*>(static_cast<char*>(array->allocation.ptr) + (i * stride));
}
const void* odTrivialArray_get_const(const odTrivialArray* array, int32_t i, int32_t stride) {
	return odTrivialArray_get(const_cast<odTrivialArray*>(array), i, stride);
}
void* odTrivialArray_begin(odTrivialArray* array) {
	if (!OD_DEBUG_CHECK(odTrivialArray_check_valid(array))) {
		return nullptr;
	}

	return array->allocation.ptr;
}
const void* odTrivialArray_begin_const(const odTrivialArray* array) {
	return odTrivialArray_begin(const_cast<odTrivialArray*>(array));
}
void* odTrivialArray_end(odTrivialArray* array, int32_t stride) {
	if (!OD_DEBUG_CHECK(odTrivialArray_check_valid(array))
		|| !OD_DEBUG_CHECK((array->count == 0) || (array->allocation.ptr != nullptr))
		|| !OD_DEBUG_CHECK(stride > 0)) {
		return nullptr;
	}

	return static_cast<void*>(static_cast<char*>(array->allocation.ptr) + (array->count * stride));
}
const void* odTrivialArray_end_const(const odTrivialArray* array, int32_t stride) {
	return odTrivialArray_end(const_cast<odTrivialArray*>(array), stride);
}
odTrivialArray::odTrivialArray()
: allocation{}, capacity{0}, count{0} {
}
odTrivialArray::odTrivialArray(odTrivialArray&& other)
: odTrivialArray{} {
	odTrivialArray_swap(this, &other);
}
odTrivialArray& odTrivialArray::operator=(odTrivialArray&& other) {
	odTrivialArray_swap(this, &other);
	return *this;
}
odTrivialArray::~odTrivialArray() {
	odTrivialArray_destroy(this);
}

const odType* odArray_get_type_constructor() {
	return odType_get<odArray>();
}
void odArray_swap(odArray* array1, odArray* array2) {
	if (!OD_DEBUG_CHECK(array1 != nullptr)
		|| !OD_DEBUG_CHECK(array2 != nullptr)) {
		return;
	}

	const odType* swap_type = array1->type;
	
	array1->type = array2->type;

	array2->type = swap_type;

	odTrivialArray_swap(&array1->array, &array2->array);
}
bool odArray_check_valid(const odArray* array) {
	if (!OD_CHECK(odTrivialArray_check_valid(&array->array))
		|| !OD_CHECK(odType_check_valid(array->type))) {
		return false;
	}

	return true;
}
const char* odArray_get_debug_string(const odArray* array) {
	if (array == nullptr) {
		return "null";
	}

	return odTrivialArray_get_debug_string(&array->array);
}
bool odArray_init(odArray* array, const odType* type) {
	if (!OD_DEBUG_CHECK(array != nullptr)
		|| !OD_DEBUG_CHECK(type != nullptr)) {
		return false;
	}

	odArray_destroy(array);

	if (!OD_CHECK(odTrivialArray_init(&array->array))) {
		return false;
	}

	array->type = type;
	return true;
}
void odArray_destroy(odArray* array) {
	if (!OD_DEBUG_CHECK(array != nullptr)) {
		return;
	}

	odTrivialArray_destroy(&array->array);
}
const odType* odArray_get_type(const odArray* array) {
	if (!OD_DEBUG_CHECK(array != nullptr)) {
		return nullptr;
	}

	return array->type;
}
int32_t odArray_get_capacity(const odArray* array) {
	if (!OD_DEBUG_CHECK(odArray_check_valid(array))) {
		return 0;
	}

	return array->array.capacity;
}
bool odArray_set_capacity(odArray* array, int32_t new_capacity) {
	if (!OD_DEBUG_CHECK(odArray_check_valid(array))
		|| !OD_DEBUG_CHECK(new_capacity >= 0)) {
		return false;
	}

	if (array->array.capacity == new_capacity) {
		return true;
	}

	odAllocation new_allocation;
	if (!OD_CHECK(odAllocation_init(&new_allocation, new_capacity * array->type->size))) {
		return false;
	}

	void* new_allocation_ptr = odAllocation_get(&new_allocation);
	if (!OD_DEBUG_CHECK((new_allocation_ptr != nullptr) || (new_capacity == 0))) {
		return false;
	}

	void* old_allocation_ptr = odAllocation_get(&array->array.allocation);
	if (!OD_DEBUG_CHECK((old_allocation_ptr != nullptr) || (array->array.capacity == 0))) {
		return false;
	}

	if ((new_allocation_ptr != nullptr) && (new_capacity > 0)) {
		array->type->default_construct_fn(new_allocation_ptr, new_capacity);
	}

	int32_t new_count = (new_capacity < array->array.count) ? new_capacity : array->array.count;
	if ((old_allocation_ptr != nullptr) && (new_allocation_ptr != nullptr) && (new_count > 0)) {
		array->type->move_assign_fn(new_allocation_ptr, old_allocation_ptr, new_count);
	}

	if ((old_allocation_ptr != nullptr) && (array->array.capacity > 0)) {
		array->type->destruct_fn(old_allocation_ptr, array->array.capacity);
	}

	odAllocation_swap(&array->array.allocation, &new_allocation);
	array->array.capacity = new_capacity;
	array->array.count = new_count;

	return true;
}
bool odArray_ensure_capacity(odArray* array, int32_t min_capacity) {
	if (!OD_DEBUG_CHECK(odArray_check_valid(array))) {
		return false;
	}

	int32_t new_capacity = odTrivialArray_get_new_capacity(array->array.capacity, min_capacity);
	if (array->array.capacity == new_capacity) {
		return true;
	}

	return odArray_set_capacity(array, new_capacity);
}
int32_t odArray_get_count(const odArray* array) {
	if (!OD_DEBUG_CHECK(odArray_check_valid(array))) {
		return 0;
	}

	return array->array.count;
}
bool odArray_set_count(odArray* array, int32_t new_count) {
	if (!OD_DEBUG_CHECK(odArray_check_valid(array))
		|| !OD_DEBUG_CHECK(new_count >= 0)) {
		return false;
	}

	if (!OD_CHECK(odArray_ensure_capacity(array, new_count))) {
		return false;
	}

	if (new_count < array->array.count) {
		int32_t popped_count = array->array.count - new_count;
		void* popped_elements = odArray_get(array, new_count);
		array->type->destruct_fn(popped_elements, popped_count);
		array->type->default_construct_fn(popped_elements, popped_count);
	}

	array->array.count = new_count;

	return true;
}
bool odArray_extend(odArray* array, void* moved_src, int32_t moved_count) {
	if (!OD_DEBUG_CHECK(odArray_check_valid(array))
		|| !OD_DEBUG_CHECK(moved_src != nullptr)
		|| !OD_DEBUG_CHECK(moved_count >= 0)) {
		return false;
	}

	if (moved_count == 0) {
		return true;
	}

	int32_t old_count = array->array.count;
	if (!OD_CHECK(odArray_set_count(array, old_count + moved_count))) {
		return false;
	}

	void* extend_dest = odArray_get(array, old_count);
	if (!OD_CHECK(extend_dest != nullptr)) {
		return false;
	}

	array->type->move_assign_fn(extend_dest, moved_src, moved_count);
	return true;
}
bool odArray_pop(odArray* array, int32_t pop_count) {
	if (!OD_CHECK(pop_count >= 0)) {
		return false;
	}

	return odArray_set_count(array, array->array.count - pop_count);
}
bool odArray_swap_pop(odArray* array, int32_t i) {
	if (!OD_DEBUG_CHECK(odArray_check_valid(array))
		|| !OD_DEBUG_CHECK((i >= 0) && (i < array->array.count))) {
		return false;
	}

	void* elem = odArray_get(array, i);
	void* last_elem = odArray_get(array, array->array.count - 1);

	if (!OD_DEBUG_CHECK(elem != nullptr)
		|| !OD_DEBUG_CHECK(last_elem != nullptr)) {
		return false;
	}

	array->type->move_assign_fn(elem, last_elem, 1);

	return odArray_pop(array, 1);
}
void* odArray_get(odArray* array, int32_t i) {
	if (!OD_DEBUG_CHECK(odArray_check_valid(array))) {
		return nullptr;
	}

	return odTrivialArray_get(&array->array, i, array->type->size);
}
const void* odArray_get_const(const odArray* array, int32_t i) {
	return odArray_get(const_cast<odArray*>(array), i);
}
void* odArray_begin(odArray* array) {
	if (!OD_DEBUG_CHECK(odArray_check_valid(array))) {
		return nullptr;
	}

	return odTrivialArray_begin(&array->array);
}
const void* odArray_begin_const(const odArray* array) {
	return odArray_begin(const_cast<odArray*>(array));
}
void* odArray_end(odArray* array) {
	if (!OD_DEBUG_CHECK(odArray_check_valid(array))) {
		return nullptr;
	}

	return odTrivialArray_end(&array->array, array->type->size);
}
const void* odArray_end_const(const odArray* array) {
	return odArray_end(const_cast<odArray*>(array));
}
odArray::odArray()
: array{}, type{nullptr} {
}
odArray::odArray(const odType* in_type)
: odArray{} {
	OD_ASSERT(odArray_init(this, in_type));
}
odArray::odArray(odArray&& other)
: odArray{} {
	odArray_swap(this, &other);
}
odArray& odArray::operator=(odArray&& other) {
	odArray_swap(this, &other);
	return *this;
}
odArray::~odArray() {
	odArray_destroy(this);
}

template struct odArrayT<int32_t>;

template struct odTrivialArrayT<int32_t>;
