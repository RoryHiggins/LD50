#pragma once

#include <od/core/array.h>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/allocation.hpp>

template<typename T>
struct odTrivialArrayT;
template<typename T>
struct odArrayT;

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
	OD_NO_DISCARD int32_t
	compare(const odTrivialArrayT<T>& other) {
		return odTrivialArray_compare(this, &other, sizeof(T));
	}
	OD_NO_DISCARD bool
	set_capacity(int32_t new_capacity) {
		return odTrivialArray_set_capacity(this, new_capacity, sizeof(T));
	}
	OD_NO_DISCARD bool
	ensure_capacity(int32_t min_capacity) {
		return odTrivialArray_ensure_capacity(this, min_capacity, sizeof(T));
	}
	OD_NO_DISCARD bool
	set_count(int32_t new_count) {
		return odTrivialArray_set_count(this, new_count, sizeof(T));
	}
	OD_NO_DISCARD bool
	ensure_count(int32_t min_count) {
		return odTrivialArray_ensure_count(this, min_count, sizeof(T));
	}
	OD_NO_DISCARD bool
	extend(const T* extend_src, int32_t extend_count) {
		return odTrivialArray_extend(this, static_cast<const void*>(extend_src), extend_count, sizeof(T));
	}
	OD_NO_DISCARD bool
	push(const T& elem) {
		return extend(&elem, 1);
	}
	OD_NO_DISCARD bool
	pop(int32_t pop_count = 1) {
		return odTrivialArray_pop(this, pop_count, sizeof(T));
	}
	OD_NO_DISCARD bool
	swap_pop(int32_t i) {
		return odTrivialArray_swap_pop(this, i, sizeof(T));
	}
	OD_NO_DISCARD bool
	assign(const T* assign_src, int32_t assign_count) {
		return odTrivialArray_assign(this, assign_src, assign_count, sizeof(T));
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
		return static_cast<T*>(odTrivialArray_end(this, sizeof(T)));
	}
	OD_NO_DISCARD const T*
	end() const& {
		return static_cast<const T*>(odTrivialArray_end_const(this, sizeof(T)));
	}
	OD_NO_DISCARD T*
	get(int32_t i) & {
		return static_cast<T*>(odTrivialArray_get(this, i, sizeof(T)));
	}
	OD_NO_DISCARD const T*
	get(int32_t i) const& {
		return static_cast<const T*>(odTrivialArray_get_const(this, i, sizeof(T)));
	}
	OD_NO_DISCARD T&
	operator[](int32_t i) & {
		return *get(i);
	}
	OD_NO_DISCARD const T&
	operator[](int32_t i) const& {
		return *get(i);
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
		return *get(i);
	}
	OD_NO_DISCARD const T&
	operator[](int32_t i) const& {
		return *get(i);
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
