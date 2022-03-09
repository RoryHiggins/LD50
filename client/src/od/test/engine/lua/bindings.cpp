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
OD_TEST(odTest_odLuaBindings_odVertexArray) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local vertex_array = odClientWrapper.VertexArray.new{
			0,0,0,0, 255,0,0,255, 0,0,
			0,1,0,0, 255,0,0,255, 0,0,
			1,0,0,0, 255,0,0,255, 0,0,
		}
		vertex_array:add_vertices{
			0,0,0,0, 255,0,0,255, 0,0,
			0,1,0,0, 255,0,0,255, 0,0,
			1,0,0,0, 255,0,0,255, 0,0,
		}
		vertex_array:sort()
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST_FILTERED(odTest_odLuaBindings_odWindow, OD_TEST_FILTER_SLOW) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local window = odClientWrapper.Window.new{is_visible = false, width = 1, height = 2}
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
		while window:step() do
			frames = frames + 1
			if frames > 60 then
				window:destroy()
				window:destroy() -- re-destroy
			end
		end
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST_FILTERED(odTest_odLuaBindings_odTexture, OD_TEST_FILTER_SLOW) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local window = odClientWrapper.Window.new{is_visible = false}

		local target_width = 8
		local target_height = 32
		local texture = odClientWrapper.Texture.new{window = window, width = target_width, height = target_height}
		local width, height = texture:get_size()
		assert(width == target_width)
		assert(height == target_height)

		texture:init{window = window, width = 1, height = 1}  -- re-init

		texture:destroy()
		texture:destroy()  -- re-destroy
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST_FILTERED(odTest_odLuaBindings_odRenderTexture, OD_TEST_FILTER_SLOW) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local window = odClientWrapper.Window.new{is_visible = false}

		local target_width = 8
		local target_height = 32
		local render_texture = odClientWrapper.RenderTexture.new{
			window = window, width = target_width, height = target_height}
		local width, height = render_texture:get_size()
		assert(width == target_width)
		assert(height == target_height)

		render_texture:init{window = window, width = 1, height = 1}  -- re-init

		render_texture:destroy()
		render_texture:destroy()  -- re-destroy
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST_FILTERED(odTest_odLuaBindings_odRenderState, OD_TEST_FILTER_SLOW) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local window = odClientWrapper.Window.new{is_visible = false}
		local texture = odClientWrapper.Texture.new{window = window, width = 1, height = 1}
		local render_state = odClientWrapper.RenderState.new_ortho_2d{target = window, src = texture}
		render_state:init_ortho_2d{target = window, src = texture}
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST_FILTERED(odTest_odLuaBindings_odRenderer, OD_TEST_FILTER_SLOW) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local vertex_array = odClientWrapper.VertexArray.new{}
		vertex_array:add_vertices{
			0,0,0,0, 255,0,0,255, 0,0,
			0,1,0,0, 255,0,0,255, 0,0,
			1,0,0,0, 255,0,0,255, 0,0,
		}

		local window = odClientWrapper.Window.new{is_visible = false}

		local texture = odClientWrapper.Texture.new{window = window, width = 1, height = 1}

		local renderer = odClientWrapper.Renderer.new{window = window}
		renderer:init{window = window}  -- re-init

		local render_state = odClientWrapper.RenderState.new_ortho_2d{target = window, src = texture}

		renderer:clear{render_state = render_state, color = {255, 255, 255, 255}}
		renderer:draw_vertex_array{render_state = render_state, vertex_array = vertex_array}
		renderer:flush()

		local render_texture = odClientWrapper.RenderTexture.new{window = window, width = 32, height = 32}
		local render_to_texture_state = odClientWrapper.RenderState.new_ortho_2d{target = render_texture, src = texture}

		renderer:draw_vertex_array{render_state = render_to_texture_state, vertex_array = vertex_array}
		renderer:flush()

		local copy_to_window_state = odClientWrapper.RenderState.new_ortho_2d{target = window, src = render_texture}
		renderer:draw_texture{render_state = copy_to_window_state}

		renderer:destroy()
		renderer:destroy()  -- re-destroy
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}

OD_TEST_SUITE(
	odTestSuite_odLuaBindings,
	odTest_odLuaBindings_register,
	odTest_odLuaBindings_odVertexArray,
	odTest_odLuaBindings_odWindow,
	odTest_odLuaBindings_odTexture,
	odTest_odLuaBindings_odRenderTexture,
	odTest_odLuaBindings_odRenderState,
	odTest_odLuaBindings_odRenderer
)
