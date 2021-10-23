#pragma once

#include <od/core/containers.h>

#include <new>

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
