local Client = odClientWrapper  -- luacheck: globals odClientWrapper

local function main()
	local entity_index = Client.EntityIndex.new{}
	for i = 1, 4 do
		entity_index:set_collider(i, (8 * i), 8, (8 * i) + 8, 16)
		entity_index:set_sprite(i, 16, 24, 24, 32)
	end

	local window = Client.Window.new{is_visible = true, width = 512, height = 512}
	local texture = Client.Texture.new_from_png_file{window = window, filename = "examples/minimal/data/sprites.png"}
	local draw_to_window = Client.RenderState.new_ortho_2d{target = window, src = texture}

	local game_render_texture = Client.RenderTexture.new{window = window, width = 128, height = 128}
	local draw_to_game = Client.RenderState.new_ortho_2d{target = game_render_texture, src = texture}

	local renderer = Client.Renderer.new{window = window}

	local vertex_array = Client.VertexArray.new{}
	vertex_array:add_vertices{
		0,0,0,0, 255,0,0,255, 0,0,
		0,128,0,0, 255,0,0,255, 0,0,
		128,0,0,0, 255,0,0,255, 0,0,
	}
	vertex_array:add_triangle(128,0, 0,128, 128,128, 0,255,0,255)
	vertex_array:add_line(0,0,128,128, 0,0,0,255, 0)
	vertex_array:add_line(0,0,0,128, 0,0,0,255, 0)
	vertex_array:add_line(0,0,128,0, 0,0,0,255, 0)
	vertex_array:add_line(0,0,8,0, 0,0,255,255, 0)
	vertex_array:add_line(0,0,0,8, 0,0,255,255, 0)
	vertex_array:add_line(7,0,7,8, 0,0,255,255, 0)
	vertex_array:add_line(0,7,8,7, 0,0,255,255, 0)
	vertex_array:add_sprite(8,64,16,72, 0,8,8,16, 255,255,255,255, 1)

	vertex_array:add_rect(22,22,44,44, 0,0,255,255, 0)
	vertex_array:add_rect_outline(22,22,44,44, 255,255,0,255, 0)

	vertex_array:add_point(24,8, 255,255,0,255, 1)
	entity_index:add_to_vertex_array{vertex_array = vertex_array}

	local window_vertex_array = Client.VertexArray.new{}
	window_vertex_array:add_line(0,0,512,512, 255,255,0,255, 0)
	window_vertex_array:add_triangle(40,0, 0,40, 40,40, 255,255,0,255)

	while window:step() do
		renderer:clear{render_state = draw_to_game, color = {255, 255, 255, 255}}
		renderer:draw_vertex_array{render_state = draw_to_game, vertex_array = vertex_array}

		renderer:clear{render_state = draw_to_window, color = {255, 255, 255, 255}}
		local copy_game_to_window = Client.RenderState.new{target = window, src = game_render_texture}
		renderer:draw_texture{render_state = copy_game_to_window}
		renderer:draw_vertex_array{render_state = draw_to_window, vertex_array = window_vertex_array}

		renderer:flush()
	end
end

main()
