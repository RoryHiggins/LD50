#pragma once

#include <od/core/string.h>

#include <od/core/array.hpp>

struct odString {
	odTrivialArrayT<char> array;

	OD_CORE_MODULE OD_NO_DISCARD int32_t
	compare(const odString& other);
	OD_CORE_MODULE OD_NO_DISCARD int32_t
	get_capacity() const;
	OD_CORE_MODULE OD_NO_DISCARD bool
	set_capacity(int32_t new_capacity);
	OD_CORE_MODULE OD_NO_DISCARD bool
	ensure_capacity(int32_t min_capacity);
	OD_CORE_MODULE OD_NO_DISCARD int32_t
	get_count() const;
	OD_CORE_MODULE OD_NO_DISCARD bool
	set_count(int32_t new_count);
	OD_CORE_MODULE OD_NO_DISCARD bool
	ensure_count(int32_t min_count);
	OD_CORE_MODULE OD_NO_DISCARD bool
	extend(const char* extend_src, int32_t extend_count);
	OD_CORE_MODULE OD_NO_DISCARD bool
	extend(const char* extend_src);
	OD_CORE_MODULE OD_NO_DISCARD bool
	extend_formatted_variadic(const char* format_c_str, va_list* args);
	OD_CORE_MODULE OD_NO_DISCARD bool
	extend_formatted(const char* format_c_str, ...);
	OD_CORE_MODULE OD_NO_DISCARD bool
	push(char elem);
	OD_CORE_MODULE OD_NO_DISCARD bool
	pop(int32_t pop_count = 1);
	OD_CORE_MODULE OD_NO_DISCARD bool
	swap_pop(int32_t i);
	OD_CORE_MODULE OD_NO_DISCARD bool
	assign(const char* assign_src, int32_t assign_count);
	OD_CORE_MODULE OD_NO_DISCARD char*
	begin() &;
	OD_CORE_MODULE OD_NO_DISCARD const char*
	begin() const&;
	OD_CORE_MODULE OD_NO_DISCARD char*
	end() &;
	OD_CORE_MODULE OD_NO_DISCARD const char*
	end() const&;
	OD_CORE_MODULE OD_NO_DISCARD char*
	get(int32_t i) &;
	OD_CORE_MODULE OD_NO_DISCARD const char*
	get(int32_t i) const&;
	OD_CORE_MODULE OD_NO_DISCARD const char*
	get_c_str() const;
	OD_CORE_MODULE OD_NO_DISCARD char&
	operator[](int32_t i) &;
	OD_CORE_MODULE OD_NO_DISCARD const char&
	operator[](int32_t i) const&;

	OD_CORE_MODULE odString();
	OD_CORE_MODULE odString(odString&& other);
	OD_CORE_MODULE odString(const odString& other);
	OD_CORE_MODULE odString& operator=(odString&& other);
	OD_CORE_MODULE odString& operator=(const odString& other);
	OD_CORE_MODULE ~odString();

	// prevent member access when expiring
	char* begin() && = delete;
	const char* begin() const&& = delete;
	char* end() && = delete;
	const char* end() const&& = delete;
	char* get(int32_t i) && = delete;
	const char* get(int32_t i) const&& = delete;
	char& operator[](int32_t i) && = delete;
	const char& operator[](int32_t i) const&& = delete;
};
