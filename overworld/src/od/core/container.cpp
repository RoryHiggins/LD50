#include <od/core/container.hpp>

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <od/core/debug.h>

bool odType_get_valid(const odType* type) {
	if ((type == nullptr)
		|| (type->default_construct_fn == nullptr)
		|| (type->move_assign_fn == nullptr)
		|| (type->destruct_fn == nullptr)) {
		return false;
	}

	return true;
}
const char* odType_get_debug_string(const odType* type) {
	if (type == nullptr) {
		return "odType{this=nullptr}";
	}

	return odDebugString_format(
		"odType{this=%p, size=%d, default_construct_fn=%p, move_assign_fn=%p, destruct_fn=%p}",
		static_cast<const void*>(type),
		type->size,
		reinterpret_cast<const void*>(type->default_construct_fn),
		reinterpret_cast<const void*>(type->move_assign_fn),
		reinterpret_cast<const void*>(type->destruct_fn)
	);
}
void* odType_index(const odType* type, void* array, int32_t i) {
	if (!OD_DEBUG_CHECK(odType_get_valid(type))
		|| !OD_DEBUG_CHECK(array != nullptr)
		|| !OD_DEBUG_CHECK(i >= 0)) {
		return nullptr;
	}

	return static_cast<void*>(static_cast<char*>(array) + (type->size * i));
}
const void* odType_index_const(const odType* type, const void* array, int32_t i) {
	return odType_index(type, const_cast<void*>(array), i);
}
static void odType_char_default_construct_fn(void* ptr, int32_t count) {
	if (!OD_DEBUG_CHECK(ptr != nullptr)
		|| !OD_DEBUG_CHECK(count >= 0)) {
		return;
	}

	memset(ptr, 0, static_cast<size_t>(count));
}
static void odType_char_move_assign_fn(void* ptr, void* src_ptr, int32_t count) {
	if (!OD_DEBUG_CHECK(ptr != nullptr)
		|| !OD_DEBUG_CHECK(src_ptr != nullptr)
		|| !OD_DEBUG_CHECK(count >= 0)) {
		return;
	}

	memmove(ptr, src_ptr, static_cast<size_t>(count));
}
static void odType_char_destruct_fn(void* ptr, int32_t count) {
	if (!OD_DEBUG_CHECK(ptr != nullptr)
		|| !OD_DEBUG_CHECK(count >= 0)) {
		return;
	}

	OD_MAYBE_UNUSED(ptr);
	OD_MAYBE_UNUSED(count);
}
const odType* odType_get_char() {
	static const odType type{
		/*size*/ 1,
		/*default_construct_fn*/ &odType_char_default_construct_fn,
		/*move_assign_fn*/ &odType_char_move_assign_fn,
		/*destruct_fn*/ &odType_char_destruct_fn};
	return &type;
}

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


odRange::odRange() : ptr{nullptr}, count{0}, stride{0} {
}
odRange::odRange(void* in_ptr, int32_t in_count, int32_t in_stride)
	: ptr{in_ptr}, count{in_count}, stride{in_stride} {
	if (!OD_DEBUG_CHECK(count >= 0)
		|| !OD_DEBUG_CHECK((count > 0) || (ptr == nullptr))
		|| !OD_DEBUG_CHECK((stride > 0) || (count == 0))) {
		ptr = nullptr;
		count = 0;
		stride = 0;
	}
}


const odType* odArray_get_type_constructor(void) {
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

const odType* odString_get_type_constructor(void) {
	return odType_get<odString>();
}
bool odString_copy(odString* string, const odString* src_string) {
	OD_TRACE("string=%s, src_string=%s", odString_get_debug_string(string), odString_get_debug_string(src_string));

	if (!OD_DEBUG_CHECK(odString_get_valid(string))
		|| !OD_DEBUG_CHECK(odString_get_valid(src_string))) {
		return false;
	}

	if (!OD_CHECK(odString_set_count(string, 0))) {
		return false;
	}

	return odString_push(string, odString_get_const(src_string, 0), odString_get_count(src_string));
}
void odString_swap(odString* string1, odString* string2) {
	OD_TRACE("string1=%s, string2=%s", odString_get_debug_string(string1), odString_get_debug_string(string2));

	if (!OD_DEBUG_CHECK(string1 != nullptr)
		|| !OD_DEBUG_CHECK(string2 != nullptr)) {
		return;
	}

	int32_t swap_count = string1->count;
	int32_t swap_capacity = string1->count;

	string1->count = string2->count;
	string1->capacity = string2->capacity;

	string2->count = swap_count;
	string2->capacity = swap_capacity;

	odAllocation_swap(&string1->allocation, &string2->allocation);
}
bool odString_get_valid(const odString* string) {
	if ((string == nullptr)
		|| (string->count < 0)
		|| (string->capacity < 0)
		|| (!odAllocation_get_valid(&string->allocation))) {
		return false;
	}

	return true;
}
int32_t odString_compare(const odString* string1, const odString* string2) {
	if (!OD_DEBUG_CHECK(string1 != nullptr)
		|| !OD_DEBUG_CHECK(string2 != nullptr)) {
		return 0;
	}

	const char* string1_ptr = odString_begin_const(string1);
	const char* string2_ptr = odString_begin_const(string2);

	string1_ptr = string1_ptr ? string1_ptr : "";
	string2_ptr = string2_ptr ? string2_ptr : "";

	int32_t string1_count = odString_get_count(string1);
	int32_t string2_count = odString_get_count(string2);

	int32_t min_count = (string1_count <= string2_count) ? string1_count : string2_count;

	int32_t order = static_cast<int32_t>(strncmp(string1_ptr, string2_ptr, static_cast<size_t>(min_count)));

	const int32_t ordering_equal = 0;
	const int32_t ordering_lhs_first = -1;
	const int32_t ordering_rhs_first = 1;
	if ((order == ordering_equal) && (string1_count != string2_count)) {
		order = (string1_count <= string2_count) ? ordering_lhs_first : ordering_rhs_first;
	}

	return order;
}
const char* odString_get_debug_string(const odString* string) {
	if (string == nullptr) {
		return "odString{this=nullptr}";
	}

	return odDebugString_format(
		"odString{this=%p, allocation=%s, capacity=%d, count=%d}",
		static_cast<const void*>(string),
		odAllocation_get_debug_string(&string->allocation),
		string->capacity,
		string->count);
}
bool odString_init(odString* string) {
	OD_TRACE("string=%s", odString_get_debug_string(string));

	if (!OD_DEBUG_CHECK(string != nullptr)) {
		return false;
	}

	odString_destroy(string);

	return true;
}
void odString_destroy(odString* string) {
	OD_TRACE("string=%s", odString_get_debug_string(string));

	if (!OD_DEBUG_CHECK(string != nullptr)) {
		return;
	}

	string->count = 0;
	string->capacity = 0;
	odAllocation_destroy(&string->allocation);
}
int32_t odString_get_capacity(const odString* string) {
	if (!OD_DEBUG_CHECK(odString_get_valid(string))) {
		return 0;
	}

	return string->capacity;
}
bool odString_set_capacity(odString* string, int32_t new_capacity) {
	OD_TRACE("array=%s, new_capacity=%d", odString_get_debug_string(string), new_capacity);

	if (!OD_DEBUG_CHECK(odString_get_valid(string))
		|| !OD_DEBUG_CHECK(new_capacity >= 0)) {
		return false;
	}

	int32_t new_count = (new_capacity < string->count) ? new_capacity : string->count;
	OD_TRACE("new_count=%d", new_count);

	odAllocation new_allocation;
	if (!OD_CHECK(odAllocation_init(&new_allocation, new_capacity))) {
		return false;
	}
	OD_TRACE("new_allocation=%s", odAllocation_get_debug_string(&new_allocation));

	void* new_allocation_ptr = odAllocation_get(&new_allocation);
	if (!OD_DEBUG_CHECK((new_allocation_ptr != nullptr) || (new_capacity == 0))) {
		return false;
	}

	void* old_allocation_ptr = odAllocation_get(&string->allocation);
	if (!OD_DEBUG_CHECK((old_allocation_ptr != nullptr) || (string->capacity == 0))) {
		return false;
	}

	if ((new_allocation_ptr != nullptr) && (new_capacity > 0)) {
		memset(new_allocation_ptr, 0, static_cast<size_t>(new_capacity));
	}

	if ((old_allocation_ptr != nullptr) && (new_allocation_ptr != nullptr) && (new_count > 0)) {
		memcpy(new_allocation_ptr, old_allocation_ptr, static_cast<size_t>(new_count));
	}

	if ((old_allocation_ptr != nullptr) && (string->capacity > 0)) {
		memset(old_allocation_ptr, 0, static_cast<size_t>(string->capacity));
	}

	odAllocation_swap(&string->allocation, &new_allocation);
	string->capacity = new_capacity;
	string->count = new_count;

	return true;
}
bool odString_ensure_capacity(odString* string, int32_t min_capacity) {
	OD_TRACE("string=%s, min_capacity=%d", odString_get_debug_string(string), min_capacity);

	if (!OD_DEBUG_CHECK(odString_get_valid(string))) {
		return false;
	}

	int32_t capacity = odString_get_capacity(string);
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

	return odString_set_capacity(string, new_capacity);
}
int32_t odString_get_count(const odString* string) {
	if (!OD_DEBUG_CHECK(odString_get_valid(string))) {
		return 0;
	}

	return string->count;
}
bool odString_set_count(odString* string, int32_t new_count) {
	OD_TRACE("string=%s, new_count=%d", odString_get_debug_string(string), new_count);

	if (!OD_DEBUG_CHECK(odString_get_valid(string))
		|| !OD_DEBUG_CHECK(new_count >= 0)) {
		return false;
	}

	if (!OD_CHECK(odString_ensure_capacity(string, new_count))) {
		return false;
	}

	if (new_count < string->count) {
		int32_t default_count = string->count - new_count;
		void* popped_elements = odString_get(string, new_count);
		memset(popped_elements, 0, static_cast<size_t>(default_count));
	}

	string->count = new_count;

	return true;
}
bool odString_expand(odString* string, char** out_expand_dest, int32_t expand_count) {
	OD_TRACE(
		"string=%s, out_expand_dest=%p, expand_count=%d",
		odString_get_debug_string(string),
		static_cast<const void*>(out_expand_dest),
		expand_count);

	if (!OD_DEBUG_CHECK(odString_get_valid(string))
		|| !OD_DEBUG_CHECK(out_expand_dest != nullptr)
		|| !OD_DEBUG_CHECK(expand_count >= 0)) {
		return false;
	}

	*out_expand_dest = nullptr;

	if (expand_count == 0) {
		return true;
	}

	int32_t offset = string->count;
	if (!OD_CHECK(odString_set_count(string, string->count + expand_count))) {
		return false;
	}

	*out_expand_dest = odString_get(string, offset);
	if (!OD_CHECK(*out_expand_dest != nullptr)) {
		return false;
	}

	return true;
}
bool odString_push(odString* string, const char* str, int32_t str_count) {
	OD_TRACE(
		"array=%s, str=%p, str_count=%d",
		odString_get_debug_string(string),
		static_cast<const void*>(str),
		str_count);

	if (!OD_DEBUG_CHECK(odString_get_valid(string))
		|| !OD_DEBUG_CHECK(str != nullptr)
		|| !OD_DEBUG_CHECK(str_count > 0)) {
		return false;
	}

	char* push_dest = nullptr;
	if (!OD_CHECK(odString_expand(string, &push_dest, str_count))) {
		return false;
	}

	OD_TRACE("push_dest=%p", static_cast<const void*>(push_dest));

	memcpy(static_cast<void*>(push_dest), str, static_cast<size_t>(str_count));

	return true;
}
bool odString_push_formatted_variadic(odString* string, const char* format_c_str, va_list args) {
	OD_TRACE("string=%s, format_c_str=%s", odString_get_debug_string(string), format_c_str ? format_c_str : "<nullptr>");

	if (!OD_DEBUG_CHECK(odString_get_valid(string))
		|| !OD_DEBUG_CHECK(format_c_str != nullptr)) {
		return false;
	}

	va_list args_copy = {};
	va_copy(args_copy, args);
	int32_t added_required_count = static_cast<int32_t>(vsnprintf(nullptr, 0, format_c_str, args_copy));
	va_end(args_copy);
	if (!OD_CHECK(added_required_count >= 0)) {
		return false;
	}

	// sprintf-style calls always write null-terminated, but count in return value
	// excludes null terminator
	int32_t added_required_capacity = added_required_count + 1;
	int32_t old_count = odString_get_count(string);
	int32_t new_required_capacity = old_count + added_required_capacity;
	if (!OD_CHECK(odString_ensure_capacity(string, new_required_capacity))) {
		return false;
	}
	
	int32_t new_required_count = old_count + added_required_count;
	if (!OD_CHECK(odString_set_count(string, new_required_count))) {
		return false;
	}
	
	char* dest_str = odString_get(string, old_count);

	int32_t written_count = static_cast<int32_t>(vsnprintf(dest_str, static_cast<size_t>(added_required_capacity), format_c_str, args));
	if (!OD_CHECK(written_count >= 0)) {
		return false;
	}

	if (!OD_CHECK(written_count == added_required_count)) {
		return false;
	}

	return true;
}
bool odString_push_formatted(odString* string, const char* format_c_str, ...) {
	va_list args = {};
	va_start(args, format_c_str);
	bool result = odString_push_formatted_variadic(string, format_c_str, args);
	va_end(args);

	return result;
}
bool odString_ensure_null_terminated(odString* string) {
	OD_TRACE("string=%s", odString_get_debug_string(string));

	if (!OD_DEBUG_CHECK(odString_get_valid(string))) {
		return false;
	}

	if (odString_get_null_terminated(string)) {
		return true;
	}

	return odString_push(string, "\0", 1);
}
bool odString_get_null_terminated(const odString* string) {
	OD_TRACE("string=%s", odString_get_debug_string(string));

	if (!OD_DEBUG_CHECK(odString_get_valid(string))) {
		return false;
	}

	if (odString_get_count(string) == 0) {
		return false;
	}

	const char* terminator_ptr = odString_get_const(string, odString_get_count(string) - 1);
	if (terminator_ptr == nullptr) {
		return false;
	}

	return (*terminator_ptr == '\0');
}
char* odString_get(odString* string, int32_t i) {
	if (!OD_DEBUG_CHECK((i >= 0) && (i < string->count))) {
		return nullptr;
	}
	return odString_begin(string) + i;
}
const char* odString_get_const(const odString* string, int32_t i) {
	if (!OD_DEBUG_CHECK((i >= 0) && (i < string->count))) {
		return nullptr;
	}
	return odString_begin_const(string) + i;
}
char* odString_begin(odString* string) {
	return static_cast<char*>(string->allocation.ptr);
}
const char* odString_begin_const(const odString* string) {
	return static_cast<const char*>(string->allocation.ptr);
}
char* odString_end(odString* string) {
	return odString_begin(string) + string->count;
}
const char* odString_end_const(const odString* string) {
	return odString_begin_const(string) + string->count;
}

odString::odString() : allocation{}, capacity{0}, count{0} {
}
odString::odString(odString&& other) : odString{} {
	odString_swap(this, &other);
}
odString::odString(const odString& other) : odString{} {
	OD_ASSERT(odString_copy(this, &other));
}
odString& odString::operator=(odString&& other) {
	odString_swap(this, &other);
	return *this;
}
odString& odString::operator=(const odString& other) {
	OD_ASSERT(odString_copy(this, &other));

	return *this;
}
odString::~odString() = default;
char* odString::operator[](int32_t i) & {
	return odString_get(this, i);
}
const char* odString::operator[](int32_t i) const& {
	return odString_get_const(this, i);
}
char* odString::begin() & {
	return odString_begin(this);
}
const char* odString::begin() const& {
	return odString_begin_const(this);
}
char* odString::end() & {
	return odString_end(this);
}
const char* odString::end() const& {
	return odString_end_const(this);
}
