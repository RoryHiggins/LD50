#include <od/core.h>
#include <od/core/vector.hpp>

#include <od/core/type.hpp>

const odType* odVector_get_type_constructor(void) {
	return odType_get<odVector>();
}
void odVector_swap(odVector* vector1, odVector* vector2) {
	if (vector1 == nullptr) {
		OD_ERROR("vector1=nullptr");
		return;
	}

	if (vector2 == nullptr) {
		OD_ERROR("vector2=nullptr");
		return;
	}

	const odType* swap_type = vector1->type;
	uint32_t swap_count = vector1->count;
	uint32_t swap_capacity = vector1->count;

	vector1->type = vector2->type;
	vector1->count = vector2->count;
	vector1->capacity = vector2->capacity;

	vector2->type = swap_type;
	vector2->count = swap_count;
	vector2->capacity = swap_capacity;

	odAllocation_swap(&vector1->allocation, &vector2->allocation);
}
const char* odVector_get_debug_string(const odVector* vector) {
	if (vector == nullptr) {
		return "odVector{this=nullptr}";
	}

	return odDebugString_create_formatted(
		"odVector{this=%p, type=%s, allocation=%s, capacity=%u, count=%u}",
		static_cast<const void*>(vector),
		odType_get_debug_string(vector->type),
		odAllocation_get_debug_string(&vector->allocation),
		vector->capacity,
		vector->count
	);
}
const odType* odVector_get_type(const odVector* vector) {
	if (vector == nullptr) {
		OD_ERROR("vector=nullptr");
		return nullptr;
	}

	return vector->type;
}
void odVector_set_type(odVector* vector, const odType* type) {
	if (vector == nullptr) {
		OD_ERROR("vector=nullptr");
		return;
	}

	if (type == nullptr) {
		OD_ERROR("type=nullptr");
		return;
	}

	OD_TRACE("vector=%s, type=%s", odVector_get_debug_string(vector), odType_get_debug_string(type));

	if (vector->capacity > 0) {
		OD_TRACE(
			"releasing old allocation, vector=%s, type=%s",
			odVector_get_debug_string(vector),
			odType_get_debug_string(type)
		);
		odVector_release(vector);
	}

	vector->type = type;
}
uint32_t odVector_get_capacity(const odVector* vector) {
	if (vector == nullptr) {
		OD_ERROR("vector=nullptr");
		return 0;
	}

	return vector->capacity;
}
bool odVector_set_capacity(odVector* vector, uint32_t new_capacity) {
	if (vector == nullptr) {
		OD_ERROR("vector=nullptr");
		return false;
	}

	if (vector->type == nullptr) {
		OD_ERROR("No type associated with vector, cannot allocate");
		return false;
	}

	uint32_t new_count = (new_capacity < vector->count) ? new_capacity : vector->count;

	OD_TRACE("vector=%s, new_capacity=%u, new_count=%u", odVector_get_debug_string(vector), new_capacity, new_count);

	odAllocation new_allocation;
	if (!odAllocation_allocate(&new_allocation, new_capacity * vector->type->size)) {
		return false;
	}

	void* old_allocation_ptr = odAllocation_get(&vector->allocation);
	if ((old_allocation_ptr == nullptr) && (vector->capacity > 0)) {
		OD_ERROR("old_allocation_ptr=nullptr for nonzero capacity");
	}

	void* new_allocation_ptr = odAllocation_get(&new_allocation);
	if ((new_allocation_ptr == nullptr) && (new_capacity > 0)) {
		OD_ERROR("new_allocation_ptr=nullptr for nonzero capacity");
	}

	if ((new_allocation_ptr != nullptr) && (new_capacity > 0)) {
		vector->type->default_construct_fn(new_allocation_ptr, new_capacity);
	}

	if ((old_allocation_ptr != nullptr) && (new_allocation_ptr != nullptr) && (new_count > 0)) {
		vector->type->move_assign_fn(new_allocation_ptr, old_allocation_ptr, new_count);
	}

	if ((old_allocation_ptr != nullptr) && (vector->capacity > 0)) {
		vector->type->destruct_fn(old_allocation_ptr, vector->capacity);
	}

	odAllocation_swap(&vector->allocation, &new_allocation);
	vector->capacity = new_capacity;
	vector->count = new_count;

	return true;
}
bool odVector_ensure_capacity(odVector* vector, uint32_t min_capacity) {
	if (vector == nullptr) {
		OD_ERROR("vector=nullptr");
		return false;
	}

	OD_TRACE("vector=%s, min_capacity=%u", odVector_get_debug_string(vector), min_capacity);

	uint32_t capacity = odVector_get_capacity(vector);
	if (capacity >= min_capacity) {
		return true;
	}

	uint32_t new_capacity = min_capacity;

	if ((capacity * 2) >= min_capacity) {
		new_capacity = capacity * 2;
	}

	const uint32_t start_capacity = min_capacity;
	if (min_capacity <= start_capacity) {
		new_capacity = start_capacity;
	}

	return odVector_set_capacity(vector, new_capacity);
}
void odVector_release(odVector* vector) {
	if (vector == nullptr) {
		OD_ERROR("vector=nullptr");
		return;
	}

	OD_TRACE("vector=%s", odVector_get_debug_string(vector));

	vector->count = 0;
	vector->capacity = 0;
	odAllocation_release(&vector->allocation);
}
uint32_t odVector_get_count(const odVector* vector) {
	if (vector == nullptr) {
		OD_ERROR("vector=nullptr");
		return 0;
	}

	return vector->count;
}
bool odVector_set_count(odVector* vector, uint32_t new_count) {
	if (vector == nullptr) {
		OD_ERROR("vector=nullptr");
		return false;
	}

	if (vector->type == nullptr) {
		OD_ERROR("No type associated with vector");
		return false;
	}

	if (!odVector_ensure_capacity(vector, new_count)) {
		OD_ERROR("Failed to ensure capacity, new_count=%u", new_count);
		return false;
	}

	OD_TRACE("vector=%s, new_count=%u", odVector_get_debug_string(vector), new_count);

	if (new_count < vector->count) {
		uint32_t default_count = vector->count - new_count;
		void* popped_elements = odVector_get(vector, new_count);
		vector->type->destruct_fn(popped_elements, default_count);
		vector->type->default_construct_fn(popped_elements, default_count);
	}

	vector->count = new_count;

	return true;
}
bool odVector_push(odVector* vector, void* moved_src, uint32_t moved_count) {
	if (vector == nullptr) {
		OD_ERROR("vector=nullptr");
		return false;
	}

	if ((moved_src == nullptr) && (moved_count > 0)) {
		OD_ERROR("moved_src == nullptr");
		return false;
	}

	if (vector->type == nullptr) {
		OD_ERROR("No type associated with vector");
		return false;
	}

	uint32_t start_index = vector->count;
	if (!odVector_set_count(vector, start_index + moved_count)) {
		return false;
	}

	OD_TRACE(
		"vector=%s, moved_src=%p, moved_count=%u",
		odVector_get_debug_string(vector),
		static_cast<const void*>(moved_src),
		moved_count
	);

	if (moved_count == 0) {
		return true;
	}

	vector->type->move_assign_fn(
		odVector_get(vector, start_index),
		moved_src,
		moved_count * vector->type->size
	);

	return true;
}
bool odVector_pop(odVector* vector, uint32_t count) {
	if (vector == nullptr) {
		OD_ERROR("vector=nullptr");
		return false;
	}

	if (vector->count < count) {
		OD_ERROR("vector too small, count=%u, vector->count=%u", count, vector->count);
		return false;
	}

	if (!odVector_set_count(vector, vector->count - count)) {
		return false;
	}

	OD_TRACE("vector=%s, count=%u", odVector_get_debug_string(vector), count);

	return true;
}
bool odVector_swap_pop(odVector* vector, uint32_t i) {
	if (vector == nullptr) {
		OD_ERROR("vector=nullptr");
		return false;
	}

	OD_TRACE("vector=%s, i=%u", odVector_get_debug_string(vector), i);

	if (vector->type == nullptr) {
		OD_ERROR("No type associated with vector");
		return false;
	}

	if (i >= vector->count) {
		OD_ERROR("outside bounds, i=%u", i);
		return false;
	}

	vector->type->move_assign_fn(
		odVector_get(vector, i),
		odVector_get(vector, vector->count - 1),
		vector->type->size
	);

	return true;
}
void* odVector_get(odVector* vector, uint32_t i) {
	if (vector == nullptr) {
		OD_ERROR("vector=nullptr");
		return nullptr;
	}

	void* elements = odAllocation_get(&vector->allocation);
	if (elements == nullptr) {
		OD_ERROR("not allocated, ptr=%s, i=%u", odVector_get_debug_string(vector), i);
		return nullptr;
	}

	if (i >= vector->count) {
		OD_ERROR("out of bounds, vector=%s, i=%u", odVector_get_debug_string(vector), i);
		return nullptr;
	}

	return odType_index(vector->type, elements, i);
}
const void* odVector_get_const(const odVector* vector, uint32_t i) {
	return const_cast<void*>(odVector_get(const_cast<odVector*>(vector), i));
}

odVector::odVector()
 : allocation{}, type{nullptr}, capacity{0}, count{0} {
}
odVector::odVector(const odType* in_type)
: odVector{} {
	odVector_set_type(this, in_type);
}
odVector::odVector(odVector&& other)
: odVector{} {
	odVector_swap(this, &other);
}
odVector& odVector::operator=(odVector&& other) {
	odVector_swap(this, &other);
	return *this;
}
odVector::~odVector() {
	OD_TRACE("vector=%s", odVector_get_debug_string(this));

	odVector_release(this);

	type = nullptr;
}
