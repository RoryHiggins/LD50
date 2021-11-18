#pragma once

#include <od/core/container.h>

#include <new>

template<typename T>
struct odArrayT;

template <typename T>
void odType_default_construct_fn(void* ptr, int32_t count);

template <typename T>
void odType_move_assign_fn(void* ptr, void* src_ptr, int32_t count);

template <typename T>
OD_NO_DISCARD const odType* odType_get();

struct odAllocation {
	void* ptr;

	OD_CORE_MODULE odAllocation();
	OD_CORE_MODULE odAllocation(odAllocation&& other);
	OD_CORE_MODULE odAllocation& operator=(odAllocation&& other);
	OD_CORE_MODULE ~odAllocation();

	odAllocation(const odAllocation& other) = delete;
	odAllocation& operator=(const odAllocation& other) = delete;
};

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

struct odString {
	struct odArray array;

	OD_CORE_MODULE odString();
	OD_CORE_MODULE odString(odString&& other);
	OD_CORE_MODULE odString(const odString& other);
	OD_CORE_MODULE odString& operator=(odString&& other);
	OD_CORE_MODULE odString& operator=(const odString& other);
	OD_CORE_MODULE ~odString();

	OD_CORE_MODULE char* operator[](int32_t i) &;
	OD_CORE_MODULE const char* operator[](int32_t i) const&;
	OD_CORE_MODULE char* begin() &;
	OD_CORE_MODULE const char* begin() const&;
	OD_CORE_MODULE char* end() &;
	OD_CORE_MODULE const char* end() const&;

	// prevent member access when expiring
	char* operator[](int32_t i) && = delete;
	const char* operator[](int32_t i) const&& = delete;
	char* begin() && = delete;
	const char* begin() const&& = delete;
	char* end() && = delete;
	const char* end() const&& = delete;
};


template<typename T>
struct odArrayT : public odArray {
	inline odArrayT() : odArray{odType_get<T>()} {
	}

	inline T* operator[](int32_t i) & {
		return static_cast<T*>(odArray_get(this, i));
	}
	inline const T* operator[](int32_t i) const & {
		return static_cast<const T*>(odArray_get_const(this, i));
	}
	inline T* begin() & {
		return static_cast<T*>(odArray_begin(this));
	}
	inline const T* begin() const & {
		return static_cast<const T*>(odArray_begin_const(this));
	}
	inline T* end() & {
		return static_cast<T*>(odArray_end(this));
	}
	inline const T* end() const & {
		return static_cast<const T*>(odArray_end_const(this));
	}
	inline bool push(T&& moved_elem) & {
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


template <typename T>
void odType_default_construct_fn(void* ptr, int32_t count) {
	for (int32_t i = 0; i < count; i++) {
		new (static_cast<T*>(ptr) + i) T{};
	}
}
template <typename T>
void odType_move_assign_fn(void* ptr, void* src_ptr, int32_t count) {
	for (int32_t i = 0; i < count; i++) {
		new (static_cast<T*>(ptr) + i) T{static_cast<T&&>(static_cast<T*>(src_ptr)[i])};
	}
}
template <typename T>
void odType_destruct_fn(void* ptr, int32_t count) {
	for (int32_t i = 0; i < count; i++) {
		static_cast<T*>(ptr)[i].~T();
	}
}
template <typename T>
OD_NO_DISCARD const odType* odType_get() {
	static const odType type{
		/*size*/ sizeof(T),
		/*default_construct_fn*/ odType_default_construct_fn<T>,
		/*move_assign_fn*/ odType_move_assign_fn<T>,
		/*destruct_fn*/ odType_destruct_fn<T>,
	};
	return &type;
}
