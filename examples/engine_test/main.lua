local Client = odClientWrapper  -- luacheck: globals odClientWrapper

local function main()
	local window = Client.Window.new{width = 512, height = 512}

	local atlas = Client.TextureAtlas.new{window = window}
	local sprites_u, sprites_v = atlas:set_region_png_file{id = 0, filename = './examples/engine_test/data/sprites.png'}

	-- add some entities
	local entity_index = Client.EntityIndex.new{}
	for i = 1, 4 do
		entity_index:set_collider(i, (8 * i), 8, (8 * i) + 8, 16)
		entity_index:set_sprite(i, sprites_u + 16, sprites_v + 24, sprites_u + 24, sprites_v + 32)
	end

	local ascii_font = Client.AsciiFont.new{
		u1 = sprites_u, v1 = sprites_v + 160,
		u2 = sprites_u + 64, v2 = sprites_v + 256,
		char_w = 8, char_h = 8,
		char_first = ' ', char_last = '~',
	}

	local vertex_array = Client.VertexArray.new{}
	vertex_array:add_triangle(0,0, 0,128, 128,0, 255,0,0,255)
	vertex_array:add_triangle(128,0, 0,128, 128,128, 0,255,0,255)
	vertex_array:add_line(0,0,128,128, 0,0,0,255, 0)
	vertex_array:add_line(0,0,0,128, 0,0,0,255, 0)
	vertex_array:add_line(0,0,128,0, 0,0,0,255, 0)
	vertex_array:add_line(0,0,8,0, 0,0,255,255, 0)
	vertex_array:add_line(0,0,0,8, 0,0,255,255, 0)
	vertex_array:add_line(7,0,7,8, 0,0,255,255, 0)
	vertex_array:add_line(0,7,8,7, 0,0,255,255, 0)
	vertex_array:add_sprite(8,64,16,72, sprites_u,sprites_v+8,sprites_u+8,sprites_v+16, 255,255,255,255, 1)

	vertex_array:add_rect(22,22,44,44, 0,0,255,255, 0)
	vertex_array:add_rect_outline(22,22,44,44, 255,255,0,255, 0)

	vertex_array:add_point(24,8, 255,255,0,255, 1)
	entity_index:add_to_vertex_array{vertex_array = vertex_array}

	ascii_font:add_text_to_vertex_array{
		vertex_array = vertex_array,
		str = "hello world!",
		x = 16, y = 16, max_w = 32, max_h = 32,
		color = {0,255,0,255}, depth = 0.0,
	}

	local window_vertex_array = Client.VertexArray.new{}
	window_vertex_array:add_line(0,0,512,512, 255,255,0,255, 0)
	window_vertex_array:add_triangle(40,0, 0,40, 40,40, 255,255,0,255)

	local renderer = Client.Renderer.new{window = window}
	local game_render_texture = Client.RenderTexture.new{window = window, width = 128, height = 128}

	while window:step() do
		local draw_to_game = Client.RenderState.new_ortho_2d{target = game_render_texture, src = atlas}
		renderer:clear{render_state = draw_to_game, color = {255, 255, 255, 255}}
		renderer:draw_vertex_array{render_state = draw_to_game, vertex_array = vertex_array}

		local draw_to_window = Client.RenderState.new_ortho_2d{target = window, src = atlas}
		renderer:clear{render_state = draw_to_window, color = {255, 255, 255, 255}}

		local copy_game_to_window = Client.RenderState.new{target = window, src = game_render_texture}
		renderer:draw_texture{render_state = copy_game_to_window}
		renderer:draw_vertex_array{render_state = draw_to_window, vertex_array = window_vertex_array}
	end
end

main()
