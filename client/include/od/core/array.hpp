#pragma once

#include <od/core/array.h>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/allocation.hpp>

template<typename T>
struct odTrivialArrayT;
template<typename T>
struct odArrayT;

template<typename T>
T& odArrayT_debug_out_of_bounds_value();

struct odTrivialArray {
	OD_CORE_MODULE OD_NO_DISCARD int32_t
	get_capacity() const;
	OD_CORE_MODULE OD_NO_DISCARD int32_t
	get_count() const;

	OD_CORE_MODULE odTrivialArray();
	OD_CORE_MODULE odTrivialArray(odTrivialArray&& other);
	OD_CORE_MODULE odTrivialArray& operator=(odTrivialArray&& other);
	OD_CORE_MODULE ~odTrivialArray();

	odAllocation allocation;
	int32_t capacity;
	int32_t count;

	odTrivialArray(const odTrivialArray& other) = delete;
	odTrivialArray& operator=(const odTrivialArray& other) = delete;
};
struct odArray {
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
	pop(int32_t pop_count = 1);
	OD_CORE_MODULE OD_NO_DISCARD bool
	swap_pop(int32_t i);

	OD_CORE_MODULE odArray();
	OD_CORE_MODULE explicit odArray(const odType* in_type);
	OD_CORE_MODULE odArray(odArray&& other);
	OD_CORE_MODULE odArray& operator=(odArray&& other);
	OD_CORE_MODULE ~odArray();

	odTrivialArray array;
	const odType* type;

	odArray(const odArray& other) = delete;
	odArray& operator=(const odArray& other) = delete;
};
// T must be trivially copyable, trivially movable, trivially default-constructible
template<typename T>
struct odTrivialArrayT : public odTrivialArray {
	static constexpr OD_NO_DISCARD int32_t
	get_stride() {
		return sizeof(T);
	}
	OD_NO_DISCARD int32_t
	compare(const odTrivialArrayT<T>& other) {
		return odTrivialArray_compare(this, &other, get_stride());
	}
	OD_NO_DISCARD bool
	set_capacity(int32_t new_capacity) {
		return odTrivialArray_set_capacity(this, new_capacity, get_stride());
	}
	OD_NO_DISCARD bool
	ensure_capacity(int32_t min_capacity) {
		return odTrivialArray_ensure_capacity(this, min_capacity, get_stride());
	}
	OD_NO_DISCARD bool
	set_count(int32_t new_count) {
		return odTrivialArray_set_count(this, new_count, get_stride());
	}
	OD_NO_DISCARD bool
	ensure_count(int32_t min_count) {
		return odTrivialArray_ensure_count(this, min_count, get_stride());
	}
	OD_NO_DISCARD bool
	extend(const T* extend_src, int32_t extend_count) {
		return odTrivialArray_extend(this, static_cast<const void*>(extend_src), extend_count, get_stride());
	}
	OD_NO_DISCARD bool
	push(const T& elem) {
		return extend(&elem, 1);
	}
	OD_NO_DISCARD bool
	pop(int32_t pop_count = 1) {
		return odTrivialArray_pop(this, pop_count, get_stride());
	}
	OD_NO_DISCARD bool
	swap_pop(int32_t i) {
		return odTrivialArray_swap_pop(this, i, get_stride());
	}
	OD_NO_DISCARD bool
	assign(const T* assign_src, int32_t assign_count) {
		return odTrivialArray_assign(this, assign_src, assign_count, get_stride());
	}
	OD_NO_DISCARD T*
	begin() & {
		return static_cast<T*>(odTrivialArray_begin(this));
	}
	OD_NO_DISCARD const T*
	begin() const& {
		return static_cast<const T*>(odTrivialArray_begin_const(this));
	}
	OD_NO_DISCARD T*
	end() & {
		return static_cast<T*>(odTrivialArray_end(this, get_stride()));
	}
	OD_NO_DISCARD const T*
	end() const& {
		return static_cast<const T*>(odTrivialArray_end_const(this, get_stride()));
	}
	OD_NO_DISCARD T*
	get(int32_t i) & {
		return static_cast<T*>(odTrivialArray_get(this, i, get_stride()));
	}
	OD_NO_DISCARD const T*
	get(int32_t i) const& {
		return static_cast<const T*>(odTrivialArray_get_const(this, i, get_stride()));
	}
	OD_NO_DISCARD T&
	operator[](int32_t i) & {
		T* elem = get(i);
		if (!OD_DEBUG_CHECK(elem != nullptr)) {
			return odArrayT_debug_out_of_bounds_value<T>();
		}
		return *elem;
	}
	OD_NO_DISCARD const T&
	operator[](int32_t i) const& {
		return const_cast<odTrivialArrayT*>(this)->operator[](i);
	}

	odTrivialArrayT() = default;
	odTrivialArrayT(odTrivialArrayT&& other) = default;
	odTrivialArrayT(const odTrivialArrayT& other)
	: odTrivialArrayT{} {
		OD_DISCARD(OD_CHECK(assign(other.begin(), other.get_count())));
	}
	odTrivialArrayT& operator=(odTrivialArrayT&& other) = default;
	odTrivialArrayT& operator=(const odTrivialArrayT& other) {
		OD_DISCARD(OD_CHECK(assign(other.begin(), other.get_count())));
		return *this;
	}
	~odTrivialArrayT() = default;

	T* begin() && = delete;
	const T* begin() const&& = delete;
	T* end() && = delete;
	const T* end() const&& = delete;
	T* get(int32_t i) && = delete;
	const T* get(int32_t i) const&& = delete;
	T& operator[](int32_t i) && = delete;
	const T& operator[](int32_t i) const&& = delete;
};
template<typename T>
struct odArrayT : public odArray {
	OD_NO_DISCARD bool
	push(T elem) {
		return odArray_extend(this, &elem, 1);
	}
	OD_NO_DISCARD T*
	begin() & {
		return static_cast<T*>(odArray_begin(this));
	}
	OD_NO_DISCARD const T*
	begin() const& {
		return static_cast<const T*>(odArray_begin_const(this));
	}
	OD_NO_DISCARD T*
	end() & {
		return static_cast<T*>(odArray_end(this));
	}
	OD_NO_DISCARD const T*
	end() const& {
		return static_cast<const T*>(odArray_end_const(this));
	}
	OD_NO_DISCARD T*
	get(int32_t i) & {
		return static_cast<T*>(odArray_get(this, i));
	}
	OD_NO_DISCARD const T*
	get(int32_t i) const& {
		return static_cast<const T*>(odArray_get_const(this, i));
	}
	OD_NO_DISCARD T&
	operator[](int32_t i) & {
		T* elem = get(i);
		if (!OD_DEBUG_CHECK(elem != nullptr)) {
			return odArrayT_debug_out_of_bounds_value<T>();
		}
		return *elem;
	}
	OD_NO_DISCARD const T&
	operator[](int32_t i) const& {
		return const_cast<odArrayT*>(this)->operator[](i);
	}

	odArrayT()
	: odArray{odType_get<T>()} {
	}
	odArrayT(odArrayT&& other) = default;
	odArrayT& operator=(odArrayT&& other) = default;
	~odArrayT() = default;

	T* begin() && = delete;
	const T* begin() const&& = delete;
	T* end() && = delete;
	const T* end() const&& = delete;
	T* get(int32_t i) && = delete;
	const T* get(int32_t i) const&& = delete;
	T& operator[](int32_t i) && = delete;
	const T& operator[](int32_t i) const&& = delete;

	odArrayT(const odArrayT& other) = delete;
	odArrayT& operator=(const odArrayT& other) = delete;
};

template<typename T> T& odArrayT_debug_out_of_bounds_value() {
	// dodgy fault tolerance for the case of out-of-bounds array accesses
	static T default_elem;
	return default_elem;
}

OD_CORE_MODULE extern template struct odTrivialArrayT<char>;
OD_CORE_MODULE extern template struct odTrivialArrayT<int32_t>;
