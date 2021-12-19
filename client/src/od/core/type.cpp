#include <od/core/type.hpp>

#include <cstring>

#include <od/core/debug.h>

bool odType_check_valid(const odType* type) {
	if (!OD_CHECK(type != nullptr)
		|| !OD_CHECK(type->default_construct_fn != nullptr)
		|| !OD_CHECK(type->move_assign_fn != nullptr)
		|| !OD_CHECK(type->destruct_fn != nullptr)) {
		return false;
	}

	return true;
}
const char* odType_get_debug_string(const odType* type) {
	if (type == nullptr) {
		return "null";
	}

	return odDebugString_format("{\"size\": %d}", type->size);
}
static void odType_char_default_construct_fn(void* ptr, int32_t count) {
	if (!OD_DEBUG_CHECK(ptr != nullptr)
		|| !OD_DEBUG_CHECK(count >= 0)) {
		return;
	}

	memset(ptr, 0, static_cast<size_t>(count));
}
static void odType_char_move_assign_fn(void* ptr, void* src_ptr, int32_t count) {
	if (!OD_DEBUG_CHECK(ptr != nullptr)
		|| !OD_DEBUG_CHECK(src_ptr != nullptr)
		|| !OD_DEBUG_CHECK(count >= 0)) {
		return;
	}

	memmove(ptr, src_ptr, static_cast<size_t>(count));
}
static void odType_char_destruct_fn(void* ptr, int32_t count) {
	if (!OD_DEBUG_CHECK(ptr != nullptr)
		|| !OD_DEBUG_CHECK(count >= 0)) {
		return;
	}

	OD_MAYBE_UNUSED(ptr);
	OD_MAYBE_UNUSED(count);
}
const odType* odType_get_char() {
	static const odType type{
		/*size*/ 1,
		/*default_construct_fn*/ &odType_char_default_construct_fn,
		/*move_assign_fn*/ &odType_char_move_assign_fn,
		/*destruct_fn*/ &odType_char_destruct_fn};
	return &type;
}
