local Client = odClientWrapper  -- luacheck: globals odClientWrapper

local function main()
	local window = Client.Window.new{is_visible = true}
	local texture = Client.Texture.new_from_png_file{window = window, filename = "examples/minimal/data/sprites.png"}
	local renderer = Client.Renderer.new{window = window}
	local game_render_texture = Client.RenderTexture.new{window = window, width = 128, height = 128}
	local draw_to_game = Client.RenderState.new_ortho_2d{target = game_render_texture, src = texture}
	local draw_to_window = Client.RenderState.new_ortho_2d{target = window, src = texture}
	local draw_game_to_window = Client.RenderState.new{target = window, src = game_render_texture}

	local triangle = Client.VertexArray.new{
		0,0,0,0, 255,0,0,255, 0,0,
		0,128,0,0, 255,0,0,255, 0,0,
		128,0,0,0, 255,0,0,255, 0,0,
	}

	while window:step() do
		renderer:clear{render_state = draw_to_game, color = {255, 255, 255, 255}}
		renderer:draw_vertex_array{render_state = draw_to_game, vertex_array = triangle}

		renderer:clear{render_state = draw_to_window, color = {255, 255, 255, 255}}
		-- renderer:draw_vertex_array{render_state = draw_to_window, vertex_array = triangle}
		renderer:draw_texture{render_state = draw_game_to_window}

		renderer:flush()
	end
end

main()
