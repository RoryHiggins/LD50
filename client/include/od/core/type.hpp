#pragma once

#include <od/core/type.h>

#include <new>

template <typename T>
void odType_default_construct_fn(void* ptr, int32_t count) {
	for (int32_t i = 0; i < count; i++) {
		new (static_cast<T*>(ptr) + i) T{};
	}
}
template <typename T>
void odType_move_assign_fn(void* ptr, void* src_ptr, int32_t count) {
	for (int32_t i = 0; i < count; i++) {
		static_cast<T*>(ptr)[i] = static_cast<T&&>(static_cast<T*>(src_ptr)[i]);
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
		/*default_construct_fn*/ &odType_default_construct_fn<T>,
		/*move_assign_fn*/ &odType_move_assign_fn<T>,
		/*destruct_fn*/ &odType_destruct_fn<T>,
	};
	return &type;
}
