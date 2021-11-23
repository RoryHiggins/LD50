#include <od/core/vector.h>

#include <od/core/debug.h>

const char* odVector_get_debug_string(const odVector* vector) {
	if (vector == nullptr) {
		return "odVector{this=nullptr}";
	}

	return odDebugString_format(
		"odVector{this=%p, vector={%g, %g, %g, %g}}",
		static_cast<const void*>(vector),
		static_cast<double>(vector->vector[0]),
		static_cast<double>(vector->vector[1]),
		static_cast<double>(vector->vector[2]),
		static_cast<double>(vector->vector[3]));
}
