#pragma once

#include <od/core/module.h>

#include <cstdlib>
#include <cstring>

#include <od/core/debug.h>

// optimized to minimize resize cost while keeping template instantiation cost low
// T must be trivially copyable+movable+constructible (incl. no destructor, safe to memcpy and memset to 0)
template<typename T>
struct odFastArrayT {
	T* ptr;
	int32_t capacity;
	int32_t count;

	OD_NO_DISCARD T* begin() & {
		return ptr;
	}
	OD_NO_DISCARD const T* begin() const & {
		return ptr;
	}
	OD_NO_DISCARD T* end() & {
		return ptr + count;
	}
	OD_NO_DISCARD const T* end() const & {
		return ptr + count;
	}
	OD_NO_DISCARD T* operator[](int32_t i) & {
		if (!OD_DEBUG_CHECK((i >= 0) && (i < count))) {
			return nullptr;
		}

		return ptr + i;
	}
	OD_NO_DISCARD const T* operator[](int32_t i) const & {
		if (!OD_DEBUG_CHECK((i >= 0) && (i < count))) {
			return nullptr;
		}

		return ptr + i;
	}

	void swap(odFastArrayT& other) {
		T* swap_ptr = ptr;
		int32_t swap_capacity = capacity;
		int32_t swap_count = count;

		ptr = other.ptr;
		capacity = other.capacity;
		count = other.count;

		other.ptr = swap_ptr;
		other.capacity = swap_capacity;
		other.count = swap_count;
	}
	OD_NO_DISCARD bool set_capacity(int32_t new_capacity) {
		if (new_capacity == 0) {
			free(ptr);
			ptr = nullptr;
			capacity = 0;
			count = 0;

			return true;
		}

		int32_t min_start_capacity = 16;
		if (new_capacity < min_start_capacity) {
			new_capacity = min_start_capacity;
		}

		if (capacity == new_capacity) {
			return true;
		}
		
		T* new_ptr = static_cast<T*>(calloc(static_cast<size_t>(new_capacity), sizeof(T)));
		if (!OD_DEBUG_CHECK(new_ptr != nullptr)) {
			return false;
		}

		memcpy(
			static_cast<void*>(new_ptr),
			static_cast<const void*>(ptr),
			sizeof(T) * static_cast<size_t>(capacity));

		free(ptr);
		ptr = new_ptr;
		capacity = new_capacity;
		if (count > new_capacity) {
			count = new_capacity;
		}

		return true;
	}
	OD_NO_DISCARD bool set_count(int32_t new_count) {
		bool ok = true;
		OD_MAYBE_UNUSED(ok);

		if (new_count > capacity) {
			ok = set_capacity(new_count * 4);  // grow 4x, to minimize reallocations
		}
		else if ((new_count * 8) < capacity) {
			ok = set_capacity(new_count);  // shrink at 2x grow rate so interleaved push+pop won't allocate
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
	: ptr{nullptr}, capacity{0}, count{0} {
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

		count = 0;
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
		bool ok = true;
		OD_MAYBE_UNUSED(ok);

		ok = set_capacity(0);
		if (!OD_DEBUG_CHECK(ok)) {
			return;
		}
	}

	T* begin() && = delete;
	const T* begin() const && = delete;
	T* end() && = delete;
	const T* end() const && = delete;
};

OD_CORE_MODULE extern template struct odFastArrayT<int32_t>;
