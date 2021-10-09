#include <od/core/string.hpp>

#include <cstdio>

#include <od/core/debug.h>
#include <od/core/type.hpp>

#define OD_STRING_ERROR(STRING, ...) \
	OD_ERROR("%s", odString_get_debug_string(STRING)); \
	OD_ERROR(__VA_ARGS__)

const odType* odString_get_type_constructor(void) {
	return odType_get<odString>();
}
bool odString_copy(odString* string, const odString* src_string) {
	OD_TRACE("string=%s, src_string=%s", odString_get_debug_string(string), odString_get_debug_string(src_string));

	if (!odString_get_valid(string)) {
		OD_STRING_ERROR(string, "not valid");
		return false;
	}

	if (!odString_get_valid(src_string)) {
		OD_STRING_ERROR(src_string, "not valid");
		return false;
	}

	if (!odString_set_count(string, 0)) {
		OD_STRING_ERROR(string, "failed to set count");
		return false;
	}

	return odString_push(string, odString_get_const(src_string, 0), odString_get_count(src_string));
}
void odString_swap(odString* string1, odString* string2) {
	odArray_swap(&string1->array, &string2->array);
}
bool odString_get_valid(const odString* string) {
	if (string == nullptr) {
		return false;
	}

	return odArray_get_valid(&string->array);
}
const char* odString_get_debug_string(const odString* string) {
	if (string == nullptr) {
		return "odString{this=nullptr}";
	}

	return odDebugString_format(
		"odString{this=%p, array=%s}", static_cast<const void*>(string), odArray_get_debug_string(&string->array));
}
void odString_release(odString* string) {
	odArray_release(&string->array);
}
bool odString_set_capacity(odString* string, int32_t new_capacity) {
	return odArray_set_capacity(&string->array, new_capacity);
}
int32_t odString_get_capacity(const odString* string) {
	return odArray_get_capacity(&string->array);
}
bool odString_ensure_capacity(odString* string, int32_t min_capacity) {
	return odArray_ensure_capacity(&string->array, min_capacity);
}
bool odString_set_count(odString* string, int32_t new_count) {
	return odArray_set_count(&string->array, new_count);
}
int32_t odString_get_count(const odString* string) {
	return odArray_get_count(&string->array);
}
bool odString_push(odString* string, const char* str, int32_t str_count) {
	return odArray_push(&string->array, const_cast<void*>(static_cast<const void*>(str)), str_count);
}
bool odString_push_formatted_variadic(odString* string, const char* format_c_str, va_list args) {
	OD_TRACE("string=%s, format_c_str=%s", odString_get_debug_string(string), format_c_str ? format_c_str : "<nullptr>");

	if (!odString_get_valid(string)) {
		OD_STRING_ERROR(string, "not valid");
		return false;
	}

	if (format_c_str == nullptr) {
		OD_STRING_ERROR(string, "format_c_str=nullptr");
		return false;
	}

	va_list args_copy = {};
	va_copy(args_copy, args);
	int required_count = vsnprintf(nullptr, 0, format_c_str, args_copy);
	va_end(args_copy);
	if (required_count < 0) {
		OD_STRING_ERROR(string, "vsnprintf size estimation failed, format_c_str=%s", format_c_str);
		return false;
	}

	// sprintf-style calls always write null-terminated, but count in return value
	// excludes null terminator
	int32_t required_capacity = static_cast<int32_t>(required_count) + 1;

	int32_t old_count = odString_get_count(string);
	odString_ensure_capacity(string, old_count + required_capacity);
	odString_set_count(string, old_count + static_cast<int32_t>(required_count));
	char* dest_str = odString_get(string, old_count);

	int written_count = vsnprintf(dest_str, static_cast<size_t>(required_capacity), format_c_str, args);
	if (written_count < 0) {
		OD_STRING_ERROR(string, "vsnprintf failed, format_c_str=%s", format_c_str);
		return false;
	}

	if (written_count != required_count) {
		OD_STRING_ERROR(string,  "less bytes written than expected, format_c_str=%s", format_c_str);
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

	if (!odString_get_valid(string)) {
		OD_STRING_ERROR(string, "not valid");
		return false;
	}

	if (odString_get_null_terminated(string)) {
		return true;
	}

	return odString_push(string, "\0", 1);
}
bool odString_get_null_terminated(const odString* string) {
	OD_TRACE("string=%s", odString_get_debug_string(string));

	if (string == nullptr) {
		OD_STRING_ERROR(string, "string=nullptr");
		return false;
	}

	if (odString_get_count(string) == 0) {
		OD_TRACE("empty string, string=%s", odString_get_debug_string(string));
		return false;
	}

	const char* terminator_ptr = odString_get_const(string, odString_get_count(string) - 1);
	if (terminator_ptr == nullptr) {
		return false;
	}

	return (*terminator_ptr == '\0');
}
char* odString_get(odString* string, int32_t i) {
	return static_cast<char*>(odArray_get(&string->array, i));
}
const char* odString_get_const(const odString* string, int32_t i) {
	return static_cast<const char*>(odArray_get_const(&string->array, i));
}

odString::odString() : array{odType_get_char()} {
}
odString::odString(odString&& other) : odString{} {
	odString_swap(this, &other);
}
odString::odString(const odString& other) : odString{} {
	odString_copy(this, &other);
}
odString& odString::operator=(odString&& other) {
	odString_swap(this, &other);
	return *this;
}
odString& odString::operator=(const odString& other) {
	odString_copy(this, &other);

	return *this;
}
odString::~odString() = default;
