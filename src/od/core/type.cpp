#include <od/core/type.hpp>

#include <string.h>

#include <od/core/debug.h>

const char* odType_get_debug_string(const odType* type) {
	if (type == nullptr) {
		return "odType{this=nullptr}";
	}

	return odDebugString_format(
		"odType{this=%p, size=%u}",
		static_cast<const void*>(type),
		type->size
	);
}
void* odType_index(const odType* type, void* array, uint32_t i) {
	return static_cast<void*>(static_cast<char*>(array) + (type->size * i));
}
const void* odType_index_const(const odType* type, const void* array, uint32_t i) {
	return odType_index(type, const_cast<void*>(array), i);
}
static void odType_char_default_construct_fn(void* ptr, uint32_t count) {
	memset(ptr, 0, count);
}
static void odType_char_move_assign_fn(void* ptr, void* src_ptr, uint32_t count) {
	memmove(ptr, src_ptr, count);
}
static void odType_char_destruct_fn(void* /*ptr*/, uint32_t /*count*/) {
}
const odType* odType_get_char() {
	static const odType type{
		/*size*/ 1,
		/*default_construct_fn*/ odType_char_default_construct_fn,
		/*move_assign_fn*/ odType_char_move_assign_fn,
		/*destruct_fn*/ odType_char_destruct_fn
	};
	return &type;
}
