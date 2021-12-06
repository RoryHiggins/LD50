#pragma once

#include <od/core/module.h>

#include <cstdlib>
#include <cstring>

#include <od/core/debug.h>

// fully templated small-size optimized array for trivial types
template<typename T>
struct odFastArrayT {
	static constexpr int32_t inline_capacity = (sizeof(T) > 16) ? 1 : (16 / (sizeof(T)));
	static constexpr int32_t inline_size = static_cast<int32_t>(sizeof(T)) * inline_capacity;

	T* ptr;
	int32_t capacity;
	int32_t count;
	T inline_data[inline_capacity];

	T* begin() & {
		return ptr;
	}
	const T* begin() const & {
		return ptr;
	}
	T* end() & {
		return ptr + count;
	}
	const T* end() const & {
		return ptr + count;
	}
	T& operator[](int32_t i) & {
		if (!OD_DEBUG_CHECK((i >= 0) && (i < count))) {
			return inline_data[0];
		}

		return ptr[i];
	}
	const T& operator[](int32_t i) const & {
		if (!OD_DEBUG_CHECK((i >= 0) && (i < count))) {
			return inline_data[0];
		}

		return ptr[i];
	}

	void swap(odFastArrayT& other) {
		T* swap_ptr = ptr;
		int32_t swap_capacity = capacity;
		int32_t swap_count = count;
		T swap_inline_data[inline_capacity];
		memcpy(
			static_cast<void*>(swap_inline_data),
			static_cast<const void*>(inline_data),
			inline_size);

		ptr = other.ptr;
		capacity = other.capacity;
		count = other.count;
		memcpy(
			static_cast<void*>(inline_data),
			static_cast<const void*>(other.inline_data),
			inline_size);

		other.ptr = swap_ptr;
		other.capacity = swap_capacity;
		other.count = swap_count;
		memcpy(
			static_cast<void*>(other.inline_data),
			static_cast<const void*>(swap_inline_data),
			inline_size);
	}
	OD_NO_DISCARD bool set_capacity(int32_t new_capacity) {
		T* new_ptr = nullptr;
		if (new_capacity <= inline_capacity) {
			if (capacity <= inline_capacity) {
				return true;
			}

			new_ptr = inline_data;
			new_capacity = inline_capacity;
		} else {
			new_ptr = static_cast<T*>(calloc(static_cast<size_t>(new_capacity), sizeof(T)));
		}

		if (!OD_DEBUG_CHECK(new_ptr != nullptr)) {
			return false;
		}

		memcpy(
			static_cast<void*>(new_ptr),
			static_cast<const void*>(ptr),
			sizeof(T) * static_cast<size_t>(capacity));

		ptr = new_ptr;
		capacity = new_capacity;
		if (count > new_capacity) {
			count = new_capacity;
		}

		return true;
	}
	OD_NO_DISCARD bool set_count(int32_t new_count, bool can_shrink=true) {
		bool ok = true;
		OD_MAYBE_UNUSED(ok);

		if (new_count > capacity) {
			ok = set_capacity(new_count * 4);
		}
		else if (can_shrink && (((new_count * 4) + 1) <= capacity)) {
			ok = set_capacity(new_count);
		}

		if (!OD_DEBUG_CHECK(ok)) {
			return false;
		}

		count = new_count;
		return true;
	}
	OD_NO_DISCARD bool extend(const T* xs, int32_t xs_count) {
		int32_t old_count = count;

		bool ok = true;
		OD_MAYBE_UNUSED(ok);

		ok = set_count(count + xs_count);
		if (!OD_DEBUG_CHECK(ok)) {
			return false;
		}

		memcpy(
			static_cast<void*>(ptr + old_count),
			static_cast<const void*>(xs),
			sizeof(T) * static_cast<size_t>(xs_count));

		return true;
	}
	OD_NO_DISCARD bool swap_pop(int32_t i) {
		if (!OD_DEBUG_CHECK((i >= 0) && (i < count))) {
			return false;
		}

		if (i < (count - 1)) {
			memcpy(
				static_cast<void*>(ptr + i),
				static_cast<const void*>(ptr + count - 1),
				sizeof(T));
		}

		bool ok = true;
		OD_MAYBE_UNUSED(ok);

		ok = set_count(count - 1);
		if (!OD_DEBUG_CHECK(ok)) {
			return false;
		}

		return true;
	}

	odFastArrayT()
	: ptr{inline_data}, capacity{inline_capacity}, count{0} {
		memset(static_cast<void*>(inline_data), 0, inline_size);
	}
	odFastArrayT(odFastArrayT const& other)
	: odFastArrayT{} {
		if (!extend(other.ptr, other.count)) {
			return;
		}
	}
	odFastArrayT(odFastArrayT&& other)
	: odFastArrayT{} {
		swap(other);
	}
	odFastArrayT& operator=(const odFastArrayT& other) {
		bool ok = true;
		OD_MAYBE_UNUSED(ok);

		ok = set_count(0, /*can_shrink*/ false);
		if (!OD_DEBUG_CHECK(ok)) {
			return *this;
		}

		ok = extend(other.ptr, other.count);
		if (!OD_DEBUG_CHECK(ok)) {
			return *this;
		}

		return *this;
	}
	odFastArrayT& operator=(odFastArrayT&& other) {
		swap(other);
		return *this;
	}
	~odFastArrayT() {
		if ((capacity > inline_capacity) && OD_DEBUG_CHECK(ptr != nullptr)) {
			free(ptr);
		}

		capacity = inline_capacity;
		count = 0;
		ptr = inline_data;
	}
};
