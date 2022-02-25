#include <od/engine/lua_client.hpp>

#include <od/core/debug.h>
#include <od/test/test.hpp>

OD_TEST(odTest_odLuaClient_init_destroy) {
	odLuaClient lua;

	OD_ASSERT(odLuaClient_init(&lua));
	OD_ASSERT(odLuaClient_check_valid(&lua));
	odLuaClient_destroy(&lua);

	// double init
	OD_ASSERT(odLuaClient_init(&lua));
	OD_ASSERT(odLuaClient_check_valid(&lua));
	OD_ASSERT(odLuaClient_init(&lua));
	OD_ASSERT(odLuaClient_check_valid(&lua));

	// double destroy
	odLuaClient_destroy(&lua);
	odLuaClient_destroy(&lua);
}
OD_TEST(odTest_odLuaClient_get_lua) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));
	OD_ASSERT(odLuaClient_get_lua(&lua) != nullptr);
}

OD_TEST_SUITE(
	odTestSuite_odLuaClient,
	odTest_odLuaClient_init_destroy,
	odTest_odLuaClient_get_lua,
)
