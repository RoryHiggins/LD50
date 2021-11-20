#include <od/core/primitive.h>

#include <cstring>
#include <cstdint>

#include <od/test/test.hpp>

OD_TEST(odAtom, init_default) {
	OD_ASSERT(odAtom_default == 0);
	OD_ASSERT(strcmp(odAtom_get_str(odAtom_default), "") == 0);
}
OD_TEST(odAtom, init_str) {
	const char test_str[] = "hello";
	odAtom hello1 = odAtom_init_c_str(test_str);
	odAtom hello2 = odAtom_init_c_str(test_str);
	odAtom hello3 = odAtom_init_str(test_str, sizeof(test_str));

	OD_ASSERT(hello1 == hello2);
	OD_ASSERT(hello2 == hello3);
	OD_ASSERT(strcmp(odAtom_get_str(hello1), odAtom_get_str(hello2)) == 0);
	OD_ASSERT(strcmp(odAtom_get_str(hello2), odAtom_get_str(hello3)) == 0);
}
OD_TEST(odAtom, init_int) {
	OD_ASSERT(odAtom_init_int(0) != 0);
	OD_ASSERT(odAtom_init_int(1) != 0);
	OD_ASSERT(odAtom_init_int(INT32_MIN) != 0);

	for (int32_t i = 0; i < 10; i++) {
		// ensure the same value is always created on subsequent calls
		OD_ASSERT(odAtom_init_int(i) == odAtom_init_int(i));
	}
	OD_ASSERT(odAtom_init_int(1) == odAtom_init_c_str("1"));
	OD_ASSERT(odAtom_init_int(-1) == odAtom_init_c_str("-1"));
}
