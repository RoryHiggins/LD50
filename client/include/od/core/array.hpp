#pragma once

#include <od/core/array.h>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/allocation.hpp>

struct odTrivialArray {
	struct odAllocation allocation;
	int32_t capacity;
	int32_t count;

	OD_CORE_MODULE odTrivialArray();
	OD_CORE_MODULE odTrivialArray(odTrivialArray&& other);
	OD_CORE_MODULE odTrivialArray& operator=(odTrivialArray&& other);
	OD_CORE_MODULE ~odTrivialArray();

	odTrivialArray(const odTrivialArray& other) = delete;
	odTrivialArray& operator=(const odTrivialArray& other) = delete;
};
struct odArray {
	odTrivialArray array;
	const struct odType* type;

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
	OD_NO_DISCARD int32_t get_capacity() const {
		return odArray_get_capacity(this);
	}
	OD_NO_DISCARD bool set_capacity(int32_t new_capacity) {
		return odArray_set_capacity(this, new_capacity);
	}
	OD_NO_DISCARD bool ensure_capacity(int32_t min_capacity) {
		return odArray_ensure_capacity(this, min_capacity);
	}
	OD_NO_DISCARD int32_t get_count() const {
		return odArray_get_count(this);
	}
	OD_NO_DISCARD bool set_count(int32_t new_count) {
		return odArray_set_count(this, new_count);
	}
	OD_NO_DISCARD bool push(T elem) {
		return odArray_extend(this, &elem, 1);
	}
	OD_NO_DISCARD bool pop(int32_t pop_count = 1) {
		return odArray_pop(this, pop_count);
	}
	OD_NO_DISCARD bool swap_pop(int32_t i) {
		return odArray_swap_pop(this, i);
	}
	OD_NO_DISCARD T* begin() & {
		return static_cast<T*>(odArray_begin(this));
	}
	OD_NO_DISCARD const T* begin() const & {
		return static_cast<const T*>(odArray_begin_const(this));
	}
	OD_NO_DISCARD T* end() & {
		return static_cast<T*>(odArray_end(this));
	}
	OD_NO_DISCARD const T* end() const & {
		return static_cast<const T*>(odArray_end_const(this));
	}
	OD_NO_DISCARD T* operator[](int32_t i) & {
		return static_cast<T*>(odArray_get(this, i));
	}
	OD_NO_DISCARD const T* operator[](int32_t i) const & {
		return static_cast<const T*>(odArray_get_const(this, i));
	}

	odArrayT()
	: odArray{odType_get<T>()} {
	}
	odArrayT(odArrayT&& other) = default;
	odArrayT& operator=(odArrayT&& other) = default;
	~odArrayT() = default;

	T* operator[](int32_t i) && = delete;
	const T* operator[](int32_t i) const && = delete;
	T* begin() && = delete;
	const T* begin() const && = delete;
	T* end() && = delete;
	const T* end() const && = delete;
	bool push(T&& moved_elem) && = delete;

	odArrayT(const odArrayT& other) = delete;
	odArrayT& operator=(const odArrayT& other) = delete;
};

// optimized to minimize resize cost while keeping template instantiation cost low
// T must be trivially copyable+movable+constructible (incl. no destructor, safe to memcpy and memset to 0)
template<typename T>
struct odTrivialArrayT : public odTrivialArray {
	static constexpr int32_t stride = sizeof(T);

	OD_NO_DISCARD int32_t compare(const struct odTrivialArrayT<T>& other) {
		return odTrivialArray_compare(this, &other, stride);
	}
	OD_NO_DISCARD int32_t get_capacity() const {
		return odTrivialArray_get_capacity(this);
	}
	OD_NO_DISCARD bool set_capacity(int32_t new_capacity) {
		return odTrivialArray_set_capacity(this, new_capacity, stride);
	}
	OD_NO_DISCARD bool ensure_capacity(int32_t min_capacity) {
		return odTrivialArray_ensure_capacity(this, min_capacity, stride);
	}
	OD_NO_DISCARD int32_t get_count() const {
		return odTrivialArray_get_count(this);
	}
	OD_NO_DISCARD bool set_count(int32_t new_count) {
		return odTrivialArray_set_count(this, new_count, stride);
	}
	OD_NO_DISCARD bool extend(const T* extend_src, int32_t extend_count) {
		return odTrivialArray_extend(this, static_cast<const void*>(extend_src), extend_count, stride);
	}
	OD_NO_DISCARD bool push(const T& elem) {
		return extend(&elem, 1);
	}
	OD_NO_DISCARD bool pop(int32_t pop_count = 1) {
		return odTrivialArray_pop(this, pop_count, stride);
	}
	OD_NO_DISCARD bool swap_pop(int32_t i) {
		return odTrivialArray_swap_pop(this, i, stride);
	}
	OD_NO_DISCARD bool assign(const T* assign_src, int32_t assign_count) {
		return odTrivialArray_extend(this, assign_src, assign_count, stride);
	}
	OD_NO_DISCARD T* begin() & {
		return static_cast<T*>(odTrivialArray_begin(this));
	}
	OD_NO_DISCARD const T* begin() const & {
		return static_cast<const T*>(odTrivialArray_begin_const(this));
	}
	OD_NO_DISCARD T* end() & {
		return static_cast<T*>(odTrivialArray_end(this, stride));
	}
	OD_NO_DISCARD const T* end() const & {
		return static_cast<const T*>(odTrivialArray_end_const(this, stride));
	}
	OD_NO_DISCARD T* operator[](int32_t i) & {
		return static_cast<T*>(odTrivialArray_get(this, i, stride));
	}
	OD_NO_DISCARD const T* operator[](int32_t i) const & {
		return static_cast<const T*>(odTrivialArray_get_const(this, i, stride));
	}

	odTrivialArrayT() = default;
	odTrivialArrayT(odTrivialArrayT&& other) = default;
	odTrivialArrayT(const odTrivialArrayT& other)
	: odTrivialArrayT{} {
		OD_DISCARD(OD_CHECK(assign(other.begin(), other.count)));
	}
	odTrivialArrayT& operator=(odTrivialArrayT&& other) = default;
	odTrivialArrayT& operator=(const odTrivialArrayT& other) {
		OD_DISCARD(OD_CHECK(assign(other.begin(), other.count)));
		return *this;
	}
	~odTrivialArrayT() = default;

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

OD_CORE_MODULE extern template struct odTrivialArrayT<int32_t>;
