#pragma once

#include <od/core/module.h>

#include <cstring>

#include <od/core/allocation.hpp>
#include <od/core/debug.h>

// optimized to minimize resize cost while keeping template instantiation cost low
// T must be trivially copyable+movable+constructible (incl. no destructor, safe to memcpy and memset to 0)
template<typename T>
struct odFastArrayT {
	odAllocation allocation;
	int32_t capacity;
	int32_t count;

	OD_NO_DISCARD T* begin() & {
		return static_cast<T*>(allocation.ptr);
	}
	OD_NO_DISCARD const T* begin() const & {
		return static_cast<const T*>(allocation.ptr);
	}
	OD_NO_DISCARD T* end() & {
		return begin() + count;
	}
	OD_NO_DISCARD const T* end() const & {
		return begin() + count;
	}
	OD_NO_DISCARD T* operator[](int32_t i) & {
		if (!OD_DEBUG_CHECK((i >= 0) && (i < count))) {
			return nullptr;
		}

		return begin() + i;
	}
	OD_NO_DISCARD const T* operator[](int32_t i) const & {
		if (!OD_DEBUG_CHECK((i >= 0) && (i < count))) {
			return nullptr;
		}

		return begin() + i;
	}

	void swap(odFastArrayT& other) {
		int32_t swap_capacity = capacity;
		int32_t swap_count = count;

		capacity = other.capacity;
		count = other.count;

		other.capacity = swap_capacity;
		other.count = swap_count;

		odAllocation_swap(&allocation, &other.allocation);
	}
	OD_NO_DISCARD bool set_capacity(int32_t new_capacity) {
		if (new_capacity == 0) {
			odAllocation_destroy(&allocation);
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

		int32_t old_size = capacity * static_cast<int32_t>(sizeof(T));
		int32_t new_size = new_capacity * static_cast<int32_t>(sizeof(T));
		odAllocation new_allocation;
		if (!OD_CHECK(odAllocation_init(&new_allocation, new_size))) {
			return false;
		}

		int32_t moved_size = ((new_size < old_size) ? new_size : old_size);
		if ((moved_size > 0) && OD_DEBUG_CHECK(allocation.ptr != nullptr)) {
			memcpy(new_allocation.ptr, allocation.ptr, static_cast<size_t>(moved_size));
		}

		odAllocation_swap(&allocation, &new_allocation);
		capacity = new_capacity;
		if (count > new_capacity) {
			count = new_capacity;
		}

		return true;
	}
	OD_NO_DISCARD bool set_count(int32_t new_count) {
		if (new_count > capacity) {
			// grow 4x, to minimize reallocations
			if (!OD_CHECK(set_capacity(new_count * 4))) {
				return false;
			}
		} else if ((new_count * 8) < capacity) {
			 // shrink at 2x grow rate so interleaved push+pop won't allocate
			if (!OD_CHECK(set_capacity(new_count))) {
				return false;
			}
		}

		count = new_count;
		return true;
	}
	OD_NO_DISCARD bool extend(const T* xs, int32_t xs_count) {
		int32_t old_count = count;

		if (!OD_CHECK(set_count(count + xs_count))) {
			return false;
		}

		size_t xs_size = static_cast<size_t>(xs_count) * sizeof(T);
		memcpy(static_cast<void*>(begin() + old_count), static_cast<const void*>(xs), xs_size);

		return true;
	}
	OD_NO_DISCARD bool swap_pop(int32_t i) {
		if (!OD_DEBUG_CHECK((i >= 0) && (i < count))) {
			return false;
		}

		if (i < (count - 1)) {
			memcpy(static_cast<void*>(begin() + i), static_cast<const void*>(end() - 1), sizeof(T));
		}

		if (!OD_CHECK(set_count(count - 1))) {
			return false;
		}

		return true;
	}

	odFastArrayT()
	: allocation{}, capacity{0}, count{0} {
	}
	odFastArrayT(odFastArrayT const& other)
	: odFastArrayT{} {
		if (!OD_CHECK(extend(other.begin(), other.count))) {
			return;
		}
	}
	odFastArrayT(odFastArrayT&& other)
	: odFastArrayT{} {
		swap(other);
	}
	odFastArrayT& operator=(const odFastArrayT& other) {
		count = 0;
		if (!OD_CHECK(extend(other.begin(), other.count))) {
			return *this;
		}

		return *this;
	}
	odFastArrayT& operator=(odFastArrayT&& other) {
		swap(other);
		return *this;
	}
	~odFastArrayT() {
		count = 0;
		capacity = 0;
		odAllocation_destroy(&allocation);
	}

	T* begin() && = delete;
	const T* begin() const && = delete;
	T* end() && = delete;
	const T* end() const && = delete;
};

OD_CORE_MODULE extern template struct odFastArrayT<int32_t>;
