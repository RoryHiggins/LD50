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
		local window = odClientWrapper.Window.new{visible = false, width = 1, height = 2}
		window:init{visible = false, width = 3, height = 4}  -- re-init
		assert(window._metatable_name == 'Window')
		local settings = window:get_settings()
		assert(settings.width == 3)
		assert(settings.height == 4)
		assert(settings.visible == false)

		window:set_settings{width = 5, height = 6}
		local settings = window:get_settings()
		assert(settings.width == 5)
		assert(settings.height == 6)
		assert(settings.visible == false)

		local key_names = odClientWrapper.Window.get_key_names()
		assert(#key_names > 0)

		local frames = 0
		while window:step() do
			frames = frames + 1
			local mouse_state = window:get_mouse_state()
			local up_state = window:get_key_state("up")
			local down_state = window:get_key_state("down")
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
		local window = odClientWrapper.Window.new{visible = false}

		local target_width = 8
		local target_height = 32
		local texture = odClientWrapper.Texture.new{window = window, width = target_width, height = target_height}
		local width, height = texture:get_size()
		assert(width == target_width)
		assert(height == target_height)

		texture:init{window = window, width = 1, height = 1}  -- re-init

		texture:init_png_file{window = window, filename = './examples/engine_test/data/sprites.png'}

		texture:destroy()
		texture:destroy()  -- re-destroy
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST_FILTERED(odTest_odLuaBindings_odTextureAtlas, OD_TEST_FILTER_SLOW) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local window = odClientWrapper.Window.new{visible = false}

		local atlas = odClientWrapper.TextureAtlas.new{window = window}
		local width, height = atlas:get_size()
		assert(width == 0)
		assert(height == 0)

		assert(atlas:get_count() == 0)
		atlas:set_region_png_file{id = 0, filename = './examples/engine_test/data/sprites.png'}
		assert(atlas:get_count() == 1)

		atlas:set_region_png_file{id = 1, filename = './examples/engine_test/data/sprites.png'}
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

		local render_state = odClientWrapper.RenderState.new_ortho_2d{target = window}

		atlas:init{window = window}  -- re-init

		atlas:destroy()
		atlas:destroy()  -- re-destroy
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST_FILTERED(odTest_odLuaBindings_odRenderTexture, OD_TEST_FILTER_SLOW) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local window = odClientWrapper.Window.new{visible = false}

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
		local window = odClientWrapper.Window.new{visible = false}
		local render_state = odClientWrapper.RenderState.new_ortho_2d{target = window}
		render_state:init_ortho_2d{target = window}
		render_state:init{target = window}

		render_state_2 = odClientWrapper.RenderState.new{target = window}
		render_state:assign(render_state_2)

		render_state_3 = render_state_2:copy()
		local transform = {rotate_z = 0.5 * math.pi, scale_x = 2, scale_y = 2, translate_x = 2, translate_y = 4}
		render_state_3:transform_view(transform)
		render_state_3:transform_projection(transform)
		render_state_3:transform_projection{}

		render_state_3:set_viewport{viewport = {x = 5, y = 6, w = 2, h = 3}}
		render_state_3:set_viewport_ortho_2d{viewport = {x = 5, y = 6, w = 2, h = 3}, target = window}
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

		local window = odClientWrapper.Window.new{visible = false}

		local texture = odClientWrapper.Texture.new{window = window, width = 1, height = 1}

		local renderer = odClientWrapper.Renderer.new{window = window}
		renderer:init{window = window}  -- re-init

		local render_state = odClientWrapper.RenderState.new_ortho_2d{target = window}

		renderer:clear{color = {255, 255, 255, 255}}
		renderer:draw_vertex_array{render_state = render_state, src = texture, vertex_array = vertex_array}
		renderer:flush()

		local render_texture = odClientWrapper.RenderTexture.new{window = window, width = 32, height = 32}
		local render_to_texture_state = odClientWrapper.RenderState.new_ortho_2d{target = render_texture}

		renderer:draw_vertex_array{render_state = render_to_texture_state, src = texture, target = render_texture, vertex_array = vertex_array}
		renderer:flush()

		local copy_to_window_state = odClientWrapper.RenderState.new_ortho_2d{target = window}
		renderer:draw_texture{render_state = copy_to_window_state, src = texture,}

		renderer:destroy()
		renderer:destroy()  -- re-destroy
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST_FILTERED(odTest_odLuaBindings_odAudio, OD_TEST_FILTER_SLOW) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local audio = odClientWrapper.Audio.new{}
		audio:init{}
		audio:init{} -- re-init

		audio:init_wav_file{filename = 'examples/engine_test/data/1_sample_silence_22050hz_s16.wav'}
		audio:set_volume{volume = 0}

		local playback = audio:play{
			loop_count = 0, cutoff_time = 1, fadein_time = 0.001, volume = 0, loop_forever = false}
		assert(audio.is_playing(playback))

		local window = odClientWrapper.Window.new{visible = false}
		local frames = 0
		while window:step() do
			frames = frames + 1
			if frames > 10 then
				window:destroy()
				window:destroy() -- re-destroy
			end
		end

		assert(not audio.is_playing(playback))

		local playback = audio:play{}
		assert(audio.is_playing(playback))

		audio.stop(playback)
		assert(not audio.is_playing(playback))

		audio.stop_all()

		audio:destroy()
		audio:destroy() -- re-destroy
	)";

	OD_ASSERT(odLua_run_string(lua.lua, test_script, nullptr, 0));
}
OD_TEST_FILTERED(odTest_odLuaBindings_odMusic, OD_TEST_FILTER_SLOW) {
	odLuaClient lua;
	OD_ASSERT(odLuaClient_init(&lua));

	const char test_script[] = R"(
		local music = odClientWrapper.Music.new{}
		music:init{}
		music:init{} -- re-init

		music:init_file{filename = 'examples/engine_test/data/100ms_sine_440hz_22050hz_s16.ogg'}

		music:play{
			loop_count = 0, fadein_time = 0.001, volume = 0, loop_forever = false}
		assert(music.is_playing())

		local window = odClientWrapper.Window.new{visible = false}
		local frames = 0
		while window:step() do
			frames = frames + 1
			if frames > 20 then
				window:destroy()
				window:destroy() -- re-destroy
			end
		end

		assert(not music.is_playing())

		music:play{volume = 0, loop_forever = false}
		assert(music.is_playing())

		music.stop()
		assert(not music.is_playing())

		music:play{volume = 0, loop_forever = false}
		assert(music.is_playing())
		music:destroy()
		assert(not music.is_playing())

		music:destroy()
		music:destroy() -- re-destroy
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
	odTest_odLuaBindings_odTextureAtlas,
	odTest_odLuaBindings_odRenderTexture,
	odTest_odLuaBindings_odRenderState,
	odTest_odLuaBindings_odRenderer,
	odTest_odLuaBindings_odAudio,
	odTest_odLuaBindings_odMusic,
	odTest_odLuaBindings_odEntityIndex,
	odTest_odLuaBindings_odEntityIndex_odVertexArray_integration
)
