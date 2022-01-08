#include <od/core/string.hpp>

#include <cstring>
#include <cstdio>

#include <od/core/debug.h>
#include <od/core/allocation.hpp>

bool odString_check_valid(const odString* string) {
	if (!OD_CHECK(string != nullptr)
		|| !OD_CHECK(odTrivialArray_check_valid(&string->array))) {
		return false;
	}

	return true;
}
const char* odString_get_debug_string(const odString* string) {
	if (string == nullptr) {
		return "null";
	}

	const char* string_preview = "";
	int32_t string_preview_max_count = 0;
	if (string->get_count() > 0) {
		string_preview = string->begin();
		string_preview_max_count = string->get_count();

		if (string_preview != nullptr) {
			string_preview_max_count = string->get_count();
		}
	}

	if (string->get_count() > 512) {
		string_preview = "...";
		string_preview_max_count = 3;
	}

	int32_t string_preview_count = static_cast<int32_t>(strnlen(string_preview, static_cast<size_t>(string_preview_max_count)));

	return odDebugString_format("{\"count\": %d, \"data\": \"%*s\"}", string->array.count, string_preview_count, string_preview);
}
void odString_init(odString* string) {
	if (!OD_DEBUG_CHECK(string != nullptr)) {
		return;
	}

	odString_destroy(string);
}
void odString_destroy(odString* string) {
	if (!OD_DEBUG_CHECK(string != nullptr)) {
		return;
	}

	odTrivialArray_destroy(&string->array);
}
void odString_swap(odString* string1, odString* string2) {
	if (!OD_DEBUG_CHECK(string1 != nullptr)
		|| !OD_DEBUG_CHECK(string2 != nullptr)) {
		return;
	}

	odTrivialArray_swap(&string1->array, &string2->array);
}
bool odString_assign(odString* string, const char* assign_src, int32_t assign_count) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return false;
	}

	if (!OD_CHECK(odString_set_count(string, 0))) {
		return false;
	}

	return odTrivialArray_assign(&string->array, assign_src, assign_count, /*stride*/ 1);
}
int32_t odString_compare(const odString* string1, const odString* string2) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string1))
		|| !OD_DEBUG_CHECK(odString_check_valid(string2))) {
		return 0;
	}

	return odTrivialArray_compare(&string1->array, &string2->array, /*stride*/ 1);
}
int32_t odString_get_capacity(const odString* string) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return 0;
	}

	return odTrivialArray_get_capacity(&string->array);
}
bool odString_set_capacity(odString* string, int32_t new_capacity) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return false;
	}

	return odTrivialArray_set_capacity(&string->array, new_capacity, /*stride*/ 1);
}
bool odString_ensure_capacity(odString* string, int32_t min_capacity) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return false;
	}

	return odTrivialArray_ensure_capacity(&string->array, min_capacity, /*stride*/ 1);
}
int32_t odString_get_count(const odString* string) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return 0;
	}

	return odTrivialArray_get_count(&string->array);
}
bool odString_set_count(odString* string, int32_t new_count) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return false;
	}

	return odTrivialArray_set_count(&string->array, new_count, /*stride*/ 1);
}
bool odString_ensure_count(odString* string, int32_t min_count) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))
		|| !OD_DEBUG_CHECK(min_count >= 0)) {
		return false;
	}

	if (min_count <= string->array.count) {
		return true;
	}

	return odString_set_count(string, min_count);
}
bool odString_extend(odString* string, const char* extend_src, int32_t extend_count) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return false;
	}

	return odTrivialArray_extend(&string->array, static_cast<const void*>(extend_src), extend_count, /*stride*/ 1);
}
bool odString_extend_formatted_variadic(odString* string, const char* format_c_str, va_list args) {
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
bool odString_extend_formatted(odString* string, const char* format_c_str, ...) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return false;
	}

	va_list args = {};
	va_start(args, format_c_str);
	bool result = OD_CHECK(odString_extend_formatted_variadic(string, format_c_str, args));
	va_end(args);

	return result;
}
bool odString_pop(odString* string, int32_t pop_count) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return false;
	}

	return odTrivialArray_pop(&string->array, pop_count, /*stride*/ 1);
}
bool odString_swap_pop(odString* string, int32_t i) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return false;
	}

	return odTrivialArray_swap_pop(&string->array, i, /*stride*/ 1);
}
const char* odString_get_c_str(const odString* string) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return "";
	}

	const char* ptr = string->begin();
	if (ptr == nullptr) {
		return "";
	}

	return ptr;
}
char* odString_get(odString* string, int32_t i) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return nullptr;
	}

	return static_cast<char*>(odTrivialArray_get(&string->array, i, /*stride*/ 1));
}
const char* odString_get_const(const odString* string, int32_t i) {
	return odString_get(const_cast<odString*>(string), i);
}
char* odString_begin(odString* string) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return nullptr;
	}

	return static_cast<char*>(odTrivialArray_begin(&string->array));
}
const char* odString_begin_const(const odString* string) {
	return odString_begin(const_cast<odString*>(string));
}
char* odString_end(odString* string) {
	if (!OD_DEBUG_CHECK(odString_check_valid(string))) {
		return nullptr;
	}

	return static_cast<char*>(odTrivialArray_end(&string->array, /*stride*/ 1));
}
const char* odString_end_const(const odString* string) {
	return odString_end(const_cast<odString*>(string));
}
int32_t odString::compare(const odString& other) {
	return odString_compare(this, &other);
}
int32_t odString::get_capacity() const {
	return odString_get_capacity(this);
}
bool odString::set_capacity(int32_t new_capacity) {
	return odString_set_capacity(this, new_capacity);
}
bool odString::ensure_capacity(int32_t min_capacity) {
	return odString_ensure_capacity(this, min_capacity);
}
int32_t odString::get_count() const {
	return odString_get_count(this);
}
bool odString::set_count(int32_t new_count) {
	return odString_set_count(this, new_count);
}
bool odString::ensure_count(int32_t min_count) {
	return odString_ensure_count(this, min_count);
}
bool odString::extend(const char* extend_src, int32_t extend_count) {
	return odString_extend(this, extend_src, extend_count);
}
bool odString::extend_formatted_variadic(const char* format_c_str, va_list args) {
	return odString_extend_formatted(this, format_c_str, args);
}
bool odString::extend_formatted(const char* format_c_str, ...) {
	va_list args = {};
	va_start(args, format_c_str);
	bool result = odString_extend_formatted_variadic(this, format_c_str, args);
	va_end(args);

	return result;
}
bool odString::push(char elem) {
	return odString_extend(this, &elem, 1);
}
bool odString::pop(int32_t pop_count) {
	return odString_pop(this, pop_count);
}
bool odString::swap_pop(int32_t i) {
	return odString_swap_pop(this, i);
}
bool odString::assign(const char* assign_src, int32_t assign_count) {
	return odString_assign(this, assign_src, assign_count);
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
char* odString::get(int32_t i) & {
	return odString_get(this, i);
}
const char* odString::get(int32_t i) const& {
	return odString_get_const(this, i);
}
const char* odString::get_c_str() const {
	return odString_get_c_str(this);
}
char& odString::operator[](int32_t i) & {
	char* ptr = odString_get(this, i);
	if (!OD_DEBUG_CHECK(ptr != nullptr)) {
		static char default_elem;
		return default_elem;
	}
	return *ptr;
}
const char& odString::operator[](int32_t i) const& {
	return const_cast<odString*>(this)->operator[](i);
}
odString::odString() = default;
odString::odString(odString&& other) = default;
odString::odString(const odString& other) : odString{} {
	OD_DISCARD(OD_CHECK(odString_assign(this, other.begin(), other.array.get_count())));
}
odString& odString::operator=(odString&& other)  = default;
odString& odString::operator=(const odString& other) {
	OD_DISCARD(OD_CHECK(odString_assign(this, other.begin(), other.array.get_count())));

	return *this;
}
odString::~odString() = default;
