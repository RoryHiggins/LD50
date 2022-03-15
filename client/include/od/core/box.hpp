#pragma once

#include <od/core/box.h>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/allocation.hpp>

template<typename T>
struct odBoxT;

struct odBox {
	OD_CORE_MODULE odBox();
	OD_CORE_MODULE explicit odBox(const odType* in_type);
	OD_CORE_MODULE odBox(odBox&& other);
	OD_CORE_MODULE odBox& operator=(odBox&& other);
	OD_CORE_MODULE ~odBox();

	odAllocation allocation;
	const odType* type;

	odBox(const odBox& other) = delete;
	odBox& operator=(const odBox& other) = delete;
};
template<typename T>
struct odBoxT : public odBox {
	OD_NO_DISCARD T*
	get() & {
		return static_cast<T*>(odBox_get(this));
	}
	OD_NO_DISCARD const T*
	get() const& {
		return static_cast<const T*>(odBox_get_const(this));
	}

	T*
	ensure() & {
		return static_cast<T*>(odBox_ensure(this, odType_get<T>()));
	}

	odBoxT()
	: odBox{odType_get<T>()} {
	}
	odBoxT(odBoxT&& other) = default;
	odBoxT& operator=(odBoxT&& other) = default;
	~odBoxT() = default;

	T* get() && = delete;
	const T* get() const&& = delete;

	T* ensure() && = delete;

	odBoxT(const odBoxT& other) = delete;
	odBoxT& operator=(const odBoxT& other) = delete;
};
