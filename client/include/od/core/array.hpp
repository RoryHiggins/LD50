#pragma once

#include <od/core/array.h>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/allocation.hpp>

struct odArray {
	struct odAllocation allocation;
	const struct odType* type;
	int32_t capacity;
	int32_t count;

	OD_CORE_MODULE odArray();
	OD_CORE_MODULE explicit odArray(const odType* in_type);
	OD_CORE_MODULE odArray(odArray&& other);
	OD_CORE_MODULE odArray& operator=(odArray&& other);
	OD_CORE_MODULE ~odArray();

	odArray(const odArray& other) = delete;
	odArray& operator=(const odArray& other) = delete;
};
template<typename T>
struct odArrayT : public odArray {
	odArrayT() : odArray{odType_get<T>()} {
	}

	T* begin() & {
		return static_cast<T*>(odArray_begin(this));
	}
	const T* begin() const & {
		return static_cast<const T*>(odArray_begin_const(this));
	}
	T* end() & {
		return begin() + count;
	}
	const T* end() const & {
		return begin() + count;
	}
	T* operator[](int32_t i) & {
		if (!OD_DEBUG_CHECK((i >= 0) && (i < count))) {
			return nullptr;
		}
		return begin() + i;
	}
	const T* operator[](int32_t i) const & {
		if (!OD_DEBUG_CHECK((i >= 0) && (i < count))) {
			return nullptr;
		}
		return begin() + i;
	}
	bool push(T&& moved_elem) & {
		return odArray_push(this, static_cast<void*>(&moved_elem), 1);
	}

	// prevent member access when expiring
	T* operator[](int32_t i) && = delete;
	const T* operator[](int32_t i) const && = delete;
	T* begin() && = delete;
	const T* begin() const && = delete;
	T* end() && = delete;
	const T* end() const && = delete;
	bool push(T&& moved_elem) && = delete;
};

OD_CORE_MODULE extern template struct odArrayT<int32_t>;
