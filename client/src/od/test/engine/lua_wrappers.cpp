#include <od/engine/lua_wrappers.hpp>

#include <od/core/debug.h>
#include <od/core/type.hpp>
#include <od/core/string.hpp>
#include <od/platform/file.hpp>
#include <od/test/test.hpp>

#include <od/engine/lua_client.hpp>

#define OD_TEST_LUA_CLIENT_TABLE "odLuaTest_metatable"

static void odLuaTest_assert(struct lua_State* lua, const char* format_c_str, ...) {
	odString str;
	OD_ASSERT(str.extend("assert("));

	va_list args = {};
	va_start(args, format_c_str);
	OD_ASSERT(str.extend_formatted_variadic(format_c_str, &args));
	va_end(args);

	OD_ASSERT(str.extend(")"));

	OD_ASSERT(odLua_run_string(lua, str.get_c_str(), nullptr, 0));
}

OD_TEST(odTest_odLua_metatable_declare) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	odLuaTest_assert(lua.lua, "%s == nil", OD_TEST_LUA_CLIENT_TABLE);

	OD_ASSERT(odLua_metatable_declare(lua.lua, OD_TEST_LUA_CLIENT_TABLE));
	odLuaTest_assert(lua.lua, "type(%s) == 'table'", OD_TEST_LUA_CLIENT_TABLE);

	// redeclare
	OD_ASSERT(odLua_metatable_declare(lua.lua, OD_TEST_LUA_CLIENT_TABLE));
	odLuaTest_assert(lua.lua, "type(%s) == 'table'", OD_TEST_LUA_CLIENT_TABLE);

	// declare another
	OD_ASSERT(odLua_metatable_declare(lua.lua, "aaa"));
	odLuaTest_assert(lua.lua, "type(%s) == 'table'", "aaa");
}
OD_TEST(odTest_odLua_metatable_set_double) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));
	OD_ASSERT(odLua_metatable_declare(lua.lua, OD_TEST_LUA_CLIENT_TABLE));

	odLuaTest_assert(lua.lua, "%s.a == nil", OD_TEST_LUA_CLIENT_TABLE);

	OD_ASSERT(odLua_metatable_set_double(lua.lua, OD_TEST_LUA_CLIENT_TABLE, "a", 3.75));
	odLuaTest_assert(lua.lua, "%s.a == 3.75", OD_TEST_LUA_CLIENT_TABLE);

	// re-set
	OD_ASSERT(odLua_metatable_set_double(lua.lua, OD_TEST_LUA_CLIENT_TABLE, "a", -0.025));
	odLuaTest_assert(lua.lua, "%s.a == -0.025", OD_TEST_LUA_CLIENT_TABLE);

	// set multiple
	OD_ASSERT(odLua_metatable_set_double(lua.lua, OD_TEST_LUA_CLIENT_TABLE, "a", 6.5));
	OD_ASSERT(odLua_metatable_set_double(lua.lua, OD_TEST_LUA_CLIENT_TABLE, "b", 3.5));
	odLuaTest_assert(lua.lua, "%s.a + %s.b == 10", OD_TEST_LUA_CLIENT_TABLE, OD_TEST_LUA_CLIENT_TABLE);
}
OD_TEST(odTest_odLua_metatable_set_function) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));
	OD_ASSERT(odLua_metatable_declare(lua.lua, OD_TEST_LUA_CLIENT_TABLE));

	odLuaTest_assert(lua.lua, "%s.a == nil", OD_TEST_LUA_CLIENT_TABLE);

	static int call_count = 0;
	call_count = 0;

	auto test_binding = [](struct lua_State* /*state*/) -> int {
		call_count++;
		return 0;
	};
	OD_ASSERT(odLua_metatable_set_function(lua.lua, OD_TEST_LUA_CLIENT_TABLE, "a", test_binding));
	OD_ASSERT(call_count == 0);

	odLuaTest_assert(lua.lua, "%s.a() == nil", OD_TEST_LUA_CLIENT_TABLE);
	OD_ASSERT(call_count == 1);

	// multiple calls
	for (int32_t expected_call_count = 1; expected_call_count < 10; expected_call_count++) {
		OD_ASSERT(call_count == expected_call_count);
		odLuaTest_assert(lua.lua, "%s.a() == nil", OD_TEST_LUA_CLIENT_TABLE);
	}
}
OD_TEST(odTest_odLua_metatable_set_ptr) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));
	OD_ASSERT(odLua_metatable_declare(lua.lua, OD_TEST_LUA_CLIENT_TABLE));

	// set to null
	void* blah = nullptr;
	OD_ASSERT(odLua_metatable_set_ptr(lua.lua, OD_TEST_LUA_CLIENT_TABLE, "a", blah));
	odLuaTest_assert(lua.lua, "type(%s.a) == 'userdata'", OD_TEST_LUA_CLIENT_TABLE);

	// reassign to non-null
	char blah2[] = "blah";
	OD_ASSERT(odLua_metatable_set_ptr(lua.lua, OD_TEST_LUA_CLIENT_TABLE, "a", static_cast<void*>(blah2)));
}
OD_TEST(odTest_odLua_metatable_set_new_delete) {
	static int32_t construct_count = 0;
	static int32_t move_assign_count = 0;
	static int32_t destruct_count = 0;
	static int32_t test_call_count = 0;

	struct odLuaTestRAII {
		odString name;

		odLuaTestRAII() {
			construct_count++;

			OD_ASSERT(name.assign("w", 1));
		}
		odLuaTestRAII& operator=(odLuaTestRAII&& /*other*/) {
			move_assign_count++;
			return *this;
		}
		~odLuaTestRAII() {
			destruct_count++;
		}

		static int test_call(lua_State* lua) {
			// check validity of constructed object
			odLuaTestRAII* self = static_cast<odLuaTestRAII*>(odLua_function_get_userdata(lua));
			OD_ASSERT(self != nullptr);
			OD_ASSERT(odString_check_valid(&self->name));
			OD_ASSERT(self->name.begin() != nullptr);
			OD_ASSERT(*self->name.begin() == 'w');
			OD_ASSERT(self->name.extend("a"));

			test_call_count++;

			return 0;
		}
	};

	construct_count = 0;
	destruct_count = 0;
	move_assign_count = 0;
	test_call_count = 0;

	{
		odLuaClient lua;
		OD_ASSERT(odLuaClient_init(&lua));
		OD_ASSERT(odLua_metatable_declare(lua.lua, OD_TEST_LUA_CLIENT_TABLE));

		
		OD_ASSERT(odLua_metatable_set_new_delete(lua.lua, OD_TEST_LUA_CLIENT_TABLE, odType_get<odLuaTestRAII>()));
		odLuaTest_assert(lua.lua, "type(%s.new) == 'function'", OD_TEST_LUA_CLIENT_TABLE);

		// re-set
		OD_ASSERT(odLua_metatable_set_new_delete(lua.lua, OD_TEST_LUA_CLIENT_TABLE, odType_get<odLuaTestRAII>()));
		odLuaTest_assert(lua.lua, "type(%s.new) == 'function'", OD_TEST_LUA_CLIENT_TABLE);

		// instantiate
		odLuaTest_assert(lua.lua, "type(%s.new()) == 'userdata'", OD_TEST_LUA_CLIENT_TABLE);

		OD_ASSERT(construct_count == 1);
		OD_ASSERT(move_assign_count == 0);
	}

	OD_ASSERT(construct_count == 1);
	OD_ASSERT(move_assign_count == 0);
	OD_ASSERT(destruct_count == 1);  // ensure lua is destroyed for gc

	{
		odLuaClient lua;
		OD_ASSERT(odLuaClient_init(&lua));
		OD_ASSERT(odLua_metatable_declare(lua.lua, OD_TEST_LUA_CLIENT_TABLE));
		OD_ASSERT(odLua_metatable_set_new_delete(lua.lua, OD_TEST_LUA_CLIENT_TABLE, odType_get<odLuaTestRAII>()));
		OD_ASSERT(odLua_metatable_set_function(lua.lua, OD_TEST_LUA_CLIENT_TABLE, "test_call", odLuaTestRAII::test_call));

		odLuaTest_assert(lua.lua, "type(getmetatable(%s.new())) == 'table'", OD_TEST_LUA_CLIENT_TABLE);
		odLuaTest_assert(lua.lua, "type(getmetatable(%s.new()).__gc) == 'function'", OD_TEST_LUA_CLIENT_TABLE);

		OD_ASSERT(test_call_count == 0);
		odLuaTest_assert(lua.lua, "%s.new():test_call() == nil", OD_TEST_LUA_CLIENT_TABLE);
		OD_ASSERT(test_call_count == 1);

		// re-call
		odLuaTest_assert(lua.lua, "%s.new():test_call() == nil", OD_TEST_LUA_CLIENT_TABLE);
		OD_ASSERT(test_call_count == 2);
	}

	OD_ASSERT(move_assign_count == 0);
	OD_ASSERT(destruct_count == construct_count);
}
OD_TEST(odTest_odLua_metatable_set_new_delete_call_no_colon_fails) {
	auto test_call = [](lua_State* lua) -> int {
		OD_DISCARD(OD_CHECK(odLua_function_get_userdata(lua) != nullptr));

		return 0;
	};

	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));
	OD_ASSERT(odLua_metatable_declare(lua.lua, OD_TEST_LUA_CLIENT_TABLE));
	OD_ASSERT(odLua_metatable_set_new_delete(lua.lua, OD_TEST_LUA_CLIENT_TABLE, odType_get<int>()));
	OD_ASSERT(odLua_metatable_set_function(lua.lua, OD_TEST_LUA_CLIENT_TABLE, "test_call", test_call));

	odLuaTest_assert(lua.lua, "type(%s.new()) == 'userdata'", OD_TEST_LUA_CLIENT_TABLE);
	odLuaTest_assert(lua.lua, "%s.new():test_call() == nil", OD_TEST_LUA_CLIENT_TABLE);

	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(!odLua_run_string(lua.lua, OD_TEST_LUA_CLIENT_TABLE ".new().test_call()", nullptr, 0));
	}
}
OD_TEST(odTest_odLua_run_string) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	OD_ASSERT(odLua_run_string(lua.lua, "return (...)", nullptr, 0));

	const int32_t args_count = 2;
	const char* args[args_count] = {
		"hello",
		"world"
	};

	OD_ASSERT(odLua_run_string(lua.lua, "return (...)", args, args_count));
}
OD_TEST(odTest_odLua_run_string_error_fails) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(!odLua_run_string(lua.lua, "error()", nullptr, 0));
	}
}
OD_TEST(odTest_odLua_run_file) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	odString filename;
	OD_ASSERT(odTest_get_random_filename(&filename));
	odScopedTempFile temp_file{filename};

	const char* filename_str = filename.get_c_str();

	// test before writing, expect fails
	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(!odLua_run_file(lua.lua, filename_str, nullptr, 0));
	}

	const char valid_lua_str[] = "assert(true)";
	OD_ASSERT(odFile_write_all(filename_str, "w", valid_lua_str, sizeof(valid_lua_str) - 1));
	OD_ASSERT(odLua_run_file(lua.lua, filename_str, nullptr, 0));

	const char invalid_lua_str[] = "assert(false)";
	OD_ASSERT(odFile_write_all(filename_str, "w", invalid_lua_str, sizeof(invalid_lua_str) - 1));
	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(!odLua_run_file(lua.lua, filename_str, nullptr, 0));
	}
}

OD_TEST_SUITE(
	odTestSuite_odLua,
	odTest_odLua_metatable_declare,
	odTest_odLua_metatable_set_double,
	odTest_odLua_metatable_set_function,
	odTest_odLua_metatable_set_ptr,
	odTest_odLua_metatable_set_new_delete,
	odTest_odLua_metatable_set_new_delete_call_no_colon_fails,
	odTest_odLua_run_string,
	odTest_odLua_run_string_error_fails,
	odTest_odLua_run_file,
)
