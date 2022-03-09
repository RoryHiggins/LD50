local Client = odClientWrapper  -- luacheck: globals odClientWrapper

local function main()
	local window = Client.Window.new{is_visible = true, width = 512, height = 512}
	local texture = Client.Texture.new_from_png_file{window = window, filename = "examples/minimal/data/sprites.png"}
	local draw_to_window = Client.RenderState.new_ortho_2d{target = window, src = texture}

	local game_render_texture = Client.RenderTexture.new{window = window, width = 128, height = 128}
	local draw_to_game = Client.RenderState.new_ortho_2d{target = game_render_texture, src = texture}
	local copy_game_to_window = Client.RenderState.new{target = window, src = game_render_texture}

	local renderer = Client.Renderer.new{window = window}

	local vertex_array = Client.VertexArray.new{}
	vertex_array:add_vertices{
		0,0,0,0, 255,0,0,255, 0,0,
		0,128,0,0, 255,0,0,255, 0,0,
		128,0,0,0, 255,0,0,255, 0,0,
	}
	vertex_array:add_triangle(128,0, 0,128, 128,128, 0,255,0,255)
	vertex_array:add_line(0,0,256,256, 0,0,0,255, 0)
	vertex_array:add_line(1,0,1,256, 0,0,0,255, 0)
	vertex_array:add_line(0,1,256,1, 0,0,0,255, 0)
	vertex_array:add_line(0,0,8,0, 0,0,255,255, 0)
	vertex_array:add_line(0,0,0,8, 0,0,255,255, 0)
	vertex_array:add_line(7,0,7,8, 0,0,255,255, 0)
	vertex_array:add_line(0,7,8,7, 0,0,255,255, 0)
	vertex_array:add_sprite(8,8,16,16, 0,8,8,16, 255,255,255,255, 1)

	vertex_array:add_rect(22,22,44,44, 0,0,255,255, 0)
	vertex_array:add_rect_outline(22,22,44,44, 255,255,0,255, 0)

	vertex_array:add_point(24,8, 255,255,0,255, 1)

	while window:step() do
		renderer:clear{render_state = draw_to_game, color = {255, 255, 255, 255}}
		renderer:draw_vertex_array{render_state = draw_to_game, vertex_array = vertex_array}

		renderer:clear{render_state = draw_to_window, color = {255, 255, 255, 255}}
		renderer:draw_texture{render_state = copy_game_to_window}

		renderer:flush()
	end
end

main()
