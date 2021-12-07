#include <od/core/string.hpp>

#include <cstring>
#include <cstdio>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/allocation.hpp>

const odType* odString_get_type_constructor() {
	return odType_get<odString>();
}
bool odString_copy(odString* string, const odString* src_string) {
	OD_TRACE("string=%s, src_string=%s", odString_get_debug_string(string), odString_get_debug_string(src_string));

	if (!OD_DEBUG_CHECK(odString_check_valid(string))
		|| !OD_DEBUG_CHECK(odString_check_valid(src_string))) {
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
	int32_t swap_capacity = string1->capacity;

	string1->count = string2->count;
	string1->capacity = string2->capacity;

	string2->count = swap_count;
	string2->capacity = swap_capacity;

	odAllocation_swap(&string1->allocation, &string2->allocation);
}
bool odString_check_valid(const odString* string) {
	if (!OD_CHECK(string != nullptr)
		|| !OD_CHECK(string->count >= 0)
		|| !OD_CHECK(string->capacity >= 0)
		|| !OD_CHECK(odAllocation_check_valid(&string->allocation))
		|| !OD_CHECK((string->count == 0) || ((string->count + 1) <= string->capacity))
		|| !OD_CHECK((string->count == 0) || (string->allocation.ptr != nullptr))
		|| !OD_CHECK((string->count == 0)
						  || (static_cast<const char*>(string->allocation.ptr)[string->count] == '\0'))) {
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
		return "null";
	}

	const char* string_preview = "";
	int32_t string_preview_max_count = 0;
	if (string->count > 0) {
		string_preview = odString_begin_const(string);
		string_preview_max_count = string->count;

	}

	if (string->count > 4096) {
		string_preview = "...";
		string_preview_max_count = 3;
	}

	int32_t string_preview_count = static_cast<int32_t>(strnlen(string_preview, static_cast<size_t>(string_preview_max_count)));

	return odDebugString_format("{\"count\": %d, \"data\": \"%*s\"}", string->count, string_preview_count, string_preview);
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
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return 0;
	}

	return string->capacity;
}
bool odString_set_capacity(odString* string, int32_t new_capacity) {
	OD_TRACE("string=%s, new_capacity=%d", odString_get_debug_string(string), new_capacity);

	if (!OD_DEBUG_CHECK(odString_check_valid(string))
		|| !OD_DEBUG_CHECK(new_capacity >= 0)) {
		return false;
	}

	int32_t new_count = string->count;
	if (new_capacity == 0) {
		new_count = 0;
	} else if ((new_capacity - 1) < string->count) {
		new_count = new_capacity - 1;
	}
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

	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
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
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return 0;
	}

	return string->count;
}
bool odString_set_count(odString* string, int32_t new_count) {
	OD_TRACE("string=%s, new_count=%d", odString_get_debug_string(string), new_count);

	if (!OD_DEBUG_CHECK(odString_check_valid(string))
		|| !OD_DEBUG_CHECK(new_count >= 0)) {
		return false;
	}

	if (!OD_CHECK(odString_ensure_capacity(string, new_count + 1))) {
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

	if (!OD_DEBUG_CHECK(odString_check_valid(string))
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
	if (!OD_DEBUG_CHECK(*out_expand_dest != nullptr)) {
		return false;
	}

	return true;
}
bool odString_push(odString* string, const char* str, int32_t str_count) {
	OD_TRACE(
		"string=%s, str=%p, str_count=%d",
		odString_get_debug_string(string),
		static_cast<const void*>(str),
		str_count);

	if (!OD_DEBUG_CHECK(odString_check_valid(string))
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

	if (!OD_DEBUG_CHECK(odString_check_valid(string))
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
	int32_t old_count = odString_get_count(string);
	if (!OD_CHECK(odString_set_count(string, old_count + added_required_count))) {
		return false;
	}
	
	char* dest_str = odString_get(string, old_count);
	if (!OD_DEBUG_CHECK(dest_str != nullptr)) {
		return false;
	}

	int32_t written_count = static_cast<int32_t>(vsnprintf(dest_str, static_cast<size_t>(added_required_count + 1), format_c_str, args));
	if (!OD_DEBUG_CHECK(written_count >= 0)) {
		return false;
	}

	if (!OD_DEBUG_CHECK(written_count == added_required_count)) {
		return false;
	}

	OD_MAYBE_UNUSED(written_count);

	return true;
}
bool odString_push_formatted(odString* string, const char* format_c_str, ...) {
	va_list args = {};
	va_start(args, format_c_str);
	bool result = odString_push_formatted_variadic(string, format_c_str, args);
	va_end(args);

	return result;
}
const char* odString_get_c_str(const odString* string) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return "";
	}

	if (string->allocation.ptr == nullptr) {
		return "";
	}

	return static_cast<char*>(string->allocation.ptr);
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
