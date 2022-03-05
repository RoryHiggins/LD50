#include <od/engine/lua/bindings.h>

#include <od/core/debug.h>
#include <od/test/test.hpp>

#include <od/engine/lua/client.hpp>
#include <od/engine/lua/wrappers.h>

OD_TEST(odTest_odLuaBindings_register) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));
	OD_ASSERT(odLuaBindings_register(lua.lua));

	odLua_run_assert(lua.lua, "type(%s.%s) == 'table'", OD_LUA_NAMESPACE, OD_LUA_BINDINGS_WINDOW);
}
OD_TEST_FILTERED(odTest_odLuaBindings_odWindow, OD_TEST_FILTER_SLOW) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));
	OD_ASSERT(odLuaBindings_odWindow_register(lua.lua));

	const char test_script[] = R"(
		local window = od.Window.new()
		window:init{is_visible = false, width = 1, height = 2}
		window:init{is_visible = false, width = 3, height = 4}  -- re-init
		assert(window._metatable_name == 'Window')
		local settings = window:get_settings()
		assert(settings.width == 3)
		assert(settings.height == 4)
		assert(settings.is_visible == false)

		window:set_settings{width = 5, height = 6}
		local settings = window:get_settings()
		assert(settings.width == 5)
		assert(settings.height == 6)
		assert(settings.is_visible == false)

		local frames = 0
		while window:step() == true do
			frames = frames + 1
			if frames > 60 then
				window:destroy()
				window:destroy() -- re-destroy
			end
		end
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST(odTest_odLuaBindings_odTexture) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));
	OD_ASSERT(odLuaBindings_odWindow_register(lua.lua));

	const char test_script[] = R"(
		local window = od.Window.new()
		window:init{is_visible = false}

		local texture = od.Texture.new()
		local target_width = 8
		local target_height = 32
		texture:init{window = window, width = target_width, height = target_height, operation = 'init'}
		local width, height = texture:get_size()
		assert(width == target_width)
		assert(height == target_height)

		local texture2 = od.Texture.new()
		texture2:init{window = window, operation = 'init_from_png_file', filename = 'examples/minimal/data/sprites.png'}

		texture2:init{window = window, operation = 'init', width = 1, height = 1}  -- re-init

		texture:destroy()
		texture:destroy()  -- re-destroy

		window:destroy()  -- destroy before texture
		texture:destroy()
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}

OD_TEST_SUITE(
	odTestSuite_odLuaBindings,
	odTest_odLuaBindings_register,
	odTest_odLuaBindings_odWindow,
	odTest_odLuaBindings_odTexture,
)
