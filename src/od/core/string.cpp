#include <od/core.h>
#include <od/core/string.hpp>

#include <stdio.h>

#include <od/core/type.hpp>

const odType* odString_get_type_constructor(void) {
	return odType_get<odString>();
}
void odString_swap(odString* string, odString* other) {
	odVector_swap(&string->vector, &other->vector);
}
const char* odString_get_debug_string(const odString* string) {
	if (string == nullptr) {
		return "odString{this=nullptr}";
	}

	return odDebugString_create_formatted(
		"odString{this=%p, vector=%s}",
		static_cast<const void*>(string),
		odVector_get_debug_string(&string->vector)
	);
}
void odString_release(odString* string) {
	odVector_release(&string->vector);
}
bool odString_set_capacity(odString* string, uint32_t new_capacity) {
	return odVector_set_capacity(&string->vector, new_capacity);
}
uint32_t odString_get_capacity(const odString* string) {
	return odVector_get_capacity(&string->vector);
}
bool odString_ensure_capacity(odString* string, uint32_t min_capacity) {
	return odVector_ensure_capacity(&string->vector, min_capacity);
}
bool odString_set_count(odString* string, uint32_t new_count) {
	return odVector_set_count(&string->vector, new_count);
}
uint32_t odString_get_count(const odString* string) {
	return odVector_get_count(&string->vector);
}
bool odString_push(odString* string, const char* str, uint32_t str_count) {
	return odVector_push(&string->vector, const_cast<void*>(static_cast<const void*>(str)), str_count);
}
bool odString_push_formatted_variadic(odString* string, const char* format_c_str, va_list args) {
	if (string == nullptr) {
		OD_ERROR("string=nullptr");
		return false;
	}

	if (format_c_str == nullptr) {
		OD_ERROR("format_c_str=nullptr");
		return false;
	}

	int required_count = vsnprintf(nullptr, 0, format_c_str, args);
	if (required_count < 0) {
		OD_ERROR("vsnprintf size estimation failed, string=%s, format_c_str=%s", odString_get_debug_string(string), format_c_str);
		return false;
	}

	OD_TRACE("string=%s, format_c_str=%s", odString_get_debug_string(string), format_c_str);

	// sprintf-style calls always write null-terminated, but count in return value excludes null terminator
	uint32_t required_capacity = static_cast<uint32_t>(required_count) + 1;

	uint32_t old_count = odString_get_count(string);
	odString_ensure_capacity(string, old_count + required_capacity);
	odString_set_count(string, old_count + static_cast<uint32_t>(required_count));
	char* dest_str = odString_get(string, old_count);

	int written_count = vsnprintf(dest_str, static_cast<size_t>(required_capacity), format_c_str, args);
	if (written_count < 0) {
		OD_ERROR("vsnprintf failed, string=%s, format_c_str=%s", odString_get_debug_string(string), format_c_str);
		return false;
	}

	if (written_count != required_count) {
		OD_ERROR("less bytes written than expected, string=%s, format_c_str=%s", odString_get_debug_string(string), format_c_str);
		return false;
	}

	return true;
}
bool odString_push_formatted(odString* string, const char* format_c_str, ...) {
	va_list args;
	va_start(args, format_c_str);
	bool result = odString_push_formatted_variadic(string, format_c_str, args);
	va_end(args);

	return result;
}
bool odString_ensure_null_terminated(odString* string) {
	if (string == nullptr) {
		OD_ERROR("string=nullptr");
		return false;
	}

	OD_TRACE("string=%s", odString_get_debug_string(string));

	if (odString_get_null_terminated(string)) {
		return true;
	}

	return odString_push(string, "\0", 1);
}
bool odString_get_null_terminated(const odString* string) {
	if (string == nullptr) {
		OD_ERROR("string=nullptr");
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
char* odString_get(odString* string, uint32_t i) {
	return static_cast<char*>(odVector_get(&string->vector, i));
}
const char* odString_get_const(const odString* string, uint32_t i) {
	return static_cast<const char*>(odVector_get_const(&string->vector, i));
}

odString::odString()
: vector{odType_get_char()} {
}
odString::odString(odString&& other)
: odString{} {
	odString_swap(this, &other);
}
OD_API_CPP odString::odString(const odString& other)
: odString{} {
	odString_push(this, odString_get_const(&other, 0), odString_get_count(&other));
}
odString& odString::operator=(odString&& other) {
	odString_swap(this, &other);
	return *this;
}
OD_API_CPP odString& odString::operator=(const odString& other) {
	odString_set_count(this, 0);
	odString_push(this, odString_get_const(&other, 0), odString_get_count(&other));

	return *this;
}
odString::~odString() {
}
