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
		vertex_array:add_sprite(8,8,16,16, 0,8,8,16, 255,255,255,255, 0)
		vertex_array:add_rect(8,8,16,16, 255,255,255,255, 0)
		vertex_array:add_rect_outline(8,8,16,16, 255,255,255,255, 0)
		vertex_array:add_line(0,0,256,256, 255,255,255,255, 0)
		vertex_array:add_point(4,4, 255,255,255,255, 0)
		vertex_array:add_triangle(0,0, 0,1, 1,0, 0,255,0,255)
		vertex_array:sort()
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST(odTest_odLuaBindings_odAsciiFont) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local ascii_font = odClientWrapper.AsciiFont.new{
			u1 = 0,
			v1 = 160,
			u2 = 64,
			v2 = 256,
			char_w = 8,
			char_h = 8,
			char_first = ' ',
			char_last = '~',
		}

		local vertex_array = odClientWrapper.VertexArray.new{}
		ascii_font:add_text_to_vertex_array{
			vertex_array = vertex_array,
			str = "hello world",
			x = 16,
			y = 16,
		}
		ascii_font:add_text_to_vertex_array{
			vertex_array = vertex_array,
			str = "hello world",
			x = 16,
			y = 16,
			max_w = 32,
			max_h = 32,
			color = {255,255,255,255},
			depth = 0.0,
		}
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

		texture:init_png_file{window = window, filename = './examples/minimal/data/sprites.png'}

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
		--render_state:init{target = window, src = texture}
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
OD_TEST_FILTERED(odTest_odLuaBindings_odTextureAtlas, OD_TEST_FILTER_SLOW) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local window = odClientWrapper.Window.new{is_visible = false}

		local atlas = odClientWrapper.TextureAtlas.new{window = window}
		local width, height = atlas:get_size()
		assert(width == 0)
		assert(height == 0)

		assert(atlas:get_count() == 0)
		atlas:set_region_png_file{id = 0, filename = './examples/minimal/data/sprites.png'}
		assert(atlas:get_count() == 1)

		atlas:set_region_png_file{id = 1, filename = './examples/minimal/data/sprites.png'}
		assert(atlas:get_count() == 2)
		atlas:reset_region{id = 1}
		local x1,y1,x2,y2 = atlas:get_region_bounds{id = 0}
		assert(x1 < x2)
		assert(y1 < y2)

		local x1,y1,x2,y2 = atlas:get_region_bounds{id = 1}
		assert(x1 == 0)
		assert(y1 == 0)
		assert(x2 == 0)
		assert(y2 == 0)

		local render_state = odClientWrapper.RenderState.new_ortho_2d{target = window, src = atlas}

		atlas:init{window = window}  -- re-init

		atlas:destroy()
		atlas:destroy()  -- re-destroy
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST(odTest_odLuaBindings_odEntityIndex) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local entity_index = odClientWrapper.EntityIndex.new{}
		entity_index:init{} -- re-init

		entity_index:set_collider(1, 0,0,4,4, 0,1,95)

		entity_index:set_tags(1, 0,1,95)
		entity_index:set_bounds(1, 0,0,8,8)
		entity_index:set_sprite(1, 8,8,16,16, 255,255,255,255, 2, 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1)
		entity_index:set_sprite(1, 8,8,16,16, 255,255,255,255, 2)


		entity_index:set(1, 0,0,8,8, 8,8,16,16, 255,255,255,255, 2, 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1, 0,1,95)
		entity_index:set(1, 0,0,8,8, 8,8,16,16, 255,255,255,255, 2)

		local x1,y1,x2,y2, u1,v1,u2,v2, r,g,b,a, depth = entity_index:get(1)
		assert(x1 == 0)
		assert(y1 == 0)
		assert(x2 == 8)
		assert(y2 == 8)

		assert(u1 == 8)
		assert(v1 == 8)
		assert(u2 == 16)
		assert(v2 == 16)
		assert(r == 255)
		assert(g == 255)
		assert(b == 255)
		assert(a == 255)
		assert(depth == 2)

		local x1,y1,x2,y2 = entity_index:get_bounds(1)
		assert(x1 == 0)
		assert(y1 == 0)
		assert(x2 == 8)
		assert(y2 == 8)

		entity_index:set_tags(1, 0,1,95)
		local tags = {entity_index:get_tags(1)}
		assert(tags[1] == 0)
		assert(tags[2] == 1)
		assert(tags[3] == 95)
		assert(#tags == 3)

		local u1,v1,u2,v2, r,g,b,a, depth = entity_index:get_sprite(1)
		assert(u1 == 8)
		assert(v1 == 8)
		assert(u2 == 16)
		assert(v2 == 16)
		assert(r == 255)
		assert(g == 255)
		assert(b == 255)
		assert(a == 255)
		assert(depth == 2)

		local found_self = entity_index:first(nil, 0,0,8,8)
		assert(found_self == 1)

		local excluded_self_no_match = entity_index:first(1, 0,0,8,8)
		assert(excluded_self_no_match == nil)

		local empty_bounds_no_match = entity_index:first(nil, 0,0,0,0)
		assert(empty_bounds_no_match == nil)

		local all = {entity_index:all(nil, 0,0,8,8)}
		assert(#all == 1)
		assert(all[1] == 1)

		local all_exclude_self_no_match = {entity_index:all(1, 0,0,8,8)}
		assert(#all_exclude_self_no_match == 0)

		local all_empty_bounds_no_match = {entity_index:all(1, 0,0,0,0)}
		assert(#all_empty_bounds_no_match == 0)

		entity_index:set_bounds(2, 0,0,1,1)
		entity_index:set_bounds(3, 0,0,1,1)
		entity_index:set_bounds(4, 0,0,1,1)
		assert(entity_index:count(nil, 0,0,1,1) == 4)
		local all = {entity_index:all(nil, 0,0,1,1)}
		assert(#all == 4)
		for i = 1, 4 do
			local found_i = false
			for j = 1, 4 do
				if all[j] == i then
					found_i = true
				end
			end
			assert(found_i)
		end

		local all_exclude_self = {entity_index:all(1, 0,0,1,1)}
		assert(#all_exclude_self == 3)

		local all_empty_bounds_no_match = {entity_index:all(1, 0,0,0,0)}
		assert(#all_empty_bounds_no_match == 0)

		entity_index:destroy()
		entity_index:set_bounds(1, 0,0,1,1)
		entity_index:set_bounds(2, 0,0,1,1)
		entity_index:set_tags(1, 1)
		entity_index:set_tags(2, 2)

		local found_tagged = entity_index:first(nil, 0,0,1,1, 1)
		assert(found_tagged == 1)
		local found_tagged = entity_index:first(nil, 0,0,1,1, 2)
		assert(found_tagged == 2)
		assert(entity_index:count(nil, 0,0,1,1, 1) == 1)
		assert(entity_index:count(1, 0,0,1,1, 1) == 0)
		assert(entity_index:count(nil, 0,0,1,1, 2) == 1)
		assert(entity_index:count(2, 0,0,1,1, 2) == 0)

		entity_index:destroy()
		entity_index:destroy()  -- re-destroy
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST_FILTERED(odTest_odLuaBindings_odEntityIndex_odVertexArray_integration, OD_TEST_FILTER_SLOW) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local entity_index = odClientWrapper.EntityIndex.new{}
		entity_index:set(1, 0,0,8,8, 8,8,16,16, 255,255,255,255, 0, 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1, 0,1,95)

		local vertex_array = odClientWrapper.VertexArray.new{}
		entity_index:add_to_vertex_array{vertex_array = vertex_array}
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}

OD_TEST_SUITE(
	odTestSuite_odLuaBindings,
	odTest_odLuaBindings_register,
	odTest_odLuaBindings_odVertexArray,
	odTest_odLuaBindings_odAsciiFont,
	odTest_odLuaBindings_odWindow,
	odTest_odLuaBindings_odTexture,
	odTest_odLuaBindings_odRenderTexture,
	odTest_odLuaBindings_odRenderState,
	odTest_odLuaBindings_odRenderer,
	odTest_odLuaBindings_odTextureAtlas,
	odTest_odLuaBindings_odEntityIndex,
	odTest_odLuaBindings_odEntityIndex_odVertexArray_integration
)
