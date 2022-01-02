#pragma once

#include <od/core/array.h>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/allocation.hpp>

struct odTrivialArray {
	odAllocation allocation;
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
	const odType* type;

	OD_CORE_MODULE odArray();
	OD_CORE_MODULE explicit odArray(const odType* in_type);
	OD_CORE_MODULE odArray(odArray&& other);
	OD_CORE_MODULE odArray& operator=(odArray&& other);
	OD_CORE_MODULE ~odArray();

	odArray(const odArray& other) = delete;
	odArray& operator=(const odArray& other) = delete;
};
// T must be trivially copyable, trivially movable, trivially default-constructible
template<typename T>
struct odTrivialArrayT : public odTrivialArray {
	inline static constexpr OD_NO_DISCARD int32_t
	get_stride() {
		return sizeof(T);
	}
	inline OD_NO_DISCARD int32_t
	compare(const odTrivialArrayT<T>& other) {
		return odTrivialArray_compare(this, &other, get_stride());
	}
	inline OD_NO_DISCARD int32_t
	get_capacity() const {
		return odTrivialArray_get_capacity(this);
	}
	inline OD_NO_DISCARD bool
	set_capacity(int32_t new_capacity) {
		return odTrivialArray_set_capacity(this, new_capacity, get_stride());
	}
	inline OD_NO_DISCARD bool
	ensure_capacity(int32_t min_capacity) {
		return odTrivialArray_ensure_capacity(this, min_capacity, get_stride());
	}
	inline OD_NO_DISCARD int32_t
	get_count() const {
		return odTrivialArray_get_count(this);
	}
	inline OD_NO_DISCARD bool
	set_count(int32_t new_count) {
		return odTrivialArray_set_count(this, new_count, get_stride());
	}
	inline OD_NO_DISCARD bool
	ensure_count(int32_t min_count) {
		return odTrivialArray_ensure_count(this, min_count, get_stride());
	}
	inline OD_NO_DISCARD bool
	extend(const T* extend_src, int32_t extend_count) {
		return odTrivialArray_extend(this, static_cast<const void*>(extend_src), extend_count, get_stride());
	}
	inline OD_NO_DISCARD bool
	push(const T& elem) {
		return extend(&elem, 1);
	}
	inline OD_NO_DISCARD bool
	pop(int32_t pop_count = 1) {
		return odTrivialArray_pop(this, pop_count, get_stride());
	}
	inline OD_NO_DISCARD bool
	swap_pop(int32_t i) {
		return odTrivialArray_swap_pop(this, i, get_stride());
	}
	inline OD_NO_DISCARD bool
	assign(const T* assign_src, int32_t assign_count) {
		return odTrivialArray_assign(this, assign_src, assign_count, get_stride());
	}
	inline OD_NO_DISCARD T*
	begin() & {
		return static_cast<T*>(odTrivialArray_begin(this));
	}
	inline OD_NO_DISCARD const T*
	begin() const & {
		return static_cast<const T*>(odTrivialArray_begin_const(this));
	}
	inline OD_NO_DISCARD T*
	end() & {
		return static_cast<T*>(odTrivialArray_end(this, get_stride()));
	}
	inline OD_NO_DISCARD const T*
	end() const & {
		return static_cast<const T*>(odTrivialArray_end_const(this, get_stride()));
	}
	inline OD_NO_DISCARD T*
	get(int32_t i) & {
		return static_cast<T*>(odTrivialArray_get(this, i, get_stride()));
	}
	inline OD_NO_DISCARD const T*
	get(int32_t i) const & {
		return static_cast<const T*>(odTrivialArray_get_const(this, i, get_stride()));
	}
	inline OD_NO_DISCARD T&
	operator[](int32_t i) & {
		T* elem = get(i);
		if (!OD_DEBUG_CHECK(elem != nullptr)) {
			static T default_elem;
			elem = &default_elem;
		}
		return *elem;
	}
	inline OD_NO_DISCARD const T&
	operator[](int32_t i) const & {
		return const_cast<odTrivialArrayT*>(this)->operator[](i);
	}

	inline odTrivialArrayT() = default;
	inline odTrivialArrayT(odTrivialArrayT&& other) = default;
	inline odTrivialArrayT(const odTrivialArrayT& other)
	: odTrivialArrayT{} {
		OD_DISCARD(OD_CHECK(assign(other.begin(), other.count)));
	}
	inline odTrivialArrayT& operator=(odTrivialArrayT&& other) = default;
	inline odTrivialArrayT& operator=(const odTrivialArrayT& other) {
		OD_DISCARD(OD_CHECK(assign(other.begin(), other.count)));
		return *this;
	}
	inline ~odTrivialArrayT() = default;

	// prevent member access when expiring
	T* begin() && = delete;
	const T* begin() const && = delete;
	T* end() && = delete;
	const T* end() const && = delete;
	T* get(int32_t i) && = delete;
	const T* get(int32_t i) const && = delete;
	T& operator[](int32_t i) && = delete;
	const T& operator[](int32_t i) const && = delete;
};
template<typename T>
struct odArrayT : public odArray {
	inline OD_NO_DISCARD int32_t
	get_capacity() const {
		return odArray_get_capacity(this);
	}
	inline OD_NO_DISCARD bool
	set_capacity(int32_t new_capacity) {
		return odArray_set_capacity(this, new_capacity);
	}
	inline OD_NO_DISCARD bool
	ensure_capacity(int32_t min_capacity) {
		return odArray_ensure_capacity(this, min_capacity);
	}
	inline OD_NO_DISCARD int32_t
	get_count() const {
		return odArray_get_count(this);
	}
	inline OD_NO_DISCARD bool
	set_count(int32_t new_count) {
		return odArray_set_count(this, new_count);
	}
	inline OD_NO_DISCARD bool
	ensure_count(int32_t min_count) {
		return odArray_ensure_count(this, min_count);
	}
	inline OD_NO_DISCARD bool
	push(T elem) {
		return odArray_extend(this, &elem, 1);
	}
	inline OD_NO_DISCARD bool
	pop(int32_t pop_count = 1) {
		return odArray_pop(this, pop_count);
	}
	inline OD_NO_DISCARD bool
	swap_pop(int32_t i) {
		return odArray_swap_pop(this, i);
	}
	inline OD_NO_DISCARD T*
	begin() & {
		return static_cast<T*>(odArray_begin(this));
	}
	inline OD_NO_DISCARD const T*
	begin() const & {
		return static_cast<const T*>(odArray_begin_const(this));
	}
	inline OD_NO_DISCARD T*
	end() & {
		return static_cast<T*>(odArray_end(this));
	}
	inline OD_NO_DISCARD const T*
	end() const & {
		return static_cast<const T*>(odArray_end_const(this));
	}
	inline OD_NO_DISCARD T*
	get(int32_t i) & {
		return static_cast<T*>(odArray_get(this, i));
	}
	inline OD_NO_DISCARD const T*
	get(int32_t i) const & {
		return static_cast<const T*>(odArray_get_const(this, i));
	}
	inline OD_NO_DISCARD T&
	operator[](int32_t i) & {
		T* elem = get(i);
		if (!OD_DEBUG_CHECK(elem != nullptr)) {
			static T default_elem;
			elem = &default_elem;
		}
		return *elem;
	}
	inline OD_NO_DISCARD const T&
	operator[](int32_t i) const & {
		return const_cast<odArrayT*>(this)->operator[](i);
	}

	inline odArrayT()
	: odArray{odType_get<T>()} {
	}
	inline odArrayT(odArrayT&& other) = default;
	inline odArrayT& operator=(odArrayT&& other) = default;
	inline ~odArrayT() = default;

	T* begin() && = delete;
	const T* begin() const && = delete;
	T* end() && = delete;
	const T* end() const && = delete;
	T* get(int32_t i) && = delete;
	const T* get(int32_t i) const && = delete;
	T& operator[](int32_t i) && = delete;
	const T& operator[](int32_t i) const && = delete;

	odArrayT(const odArrayT& other) = delete;
	odArrayT& operator=(const odArrayT& other) = delete;
};

OD_CORE_MODULE extern template struct odTrivialArrayT<char>;
OD_CORE_MODULE extern template struct odTrivialArrayT<int32_t>;
