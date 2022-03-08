local Client = odClientWrapper  -- luacheck: globals odClientWrapper

local function main()
	local window = Client.Window.new{is_visible = true}
	local texture = Client.Texture.new_from_png_file{window = window, filename = "examples/minimal/data/sprites.png"}
	local renderer = Client.Renderer.new{window = window}
	local render_state = Client.RenderState.new_ortho_2d{target = window, src = texture}

	local triangle = Client.VertexArray.new{
		32,32,0,0, 255,0,0,255, 0,0,
		32,96,0,0, 255,0,0,255, 0,0,
		96,32,0,0, 255,0,0,255, 0,0,
	}

	while window:step() do
		renderer:clear{render_state = render_state, color = {255, 255, 255, 255}}
		renderer:draw_vertex_array{render_state = render_state, vertex_array = triangle}
		renderer:flush()
	end
end

main()
