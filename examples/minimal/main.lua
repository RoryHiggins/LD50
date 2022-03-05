local client = odClient

local function main()
	local window = client.Window.new()
	window:init{}

	local src_texture = client.Texture.new()
	src_texture:init_from_png_file{window=window, filename="examples/minimal/data/sprites.png"}

	local render_state = client.RenderState.new()
	render_state:init_ortho_2d{target=window, src=src_texture}

	local renderer = client.Renderer.new()
	renderer:init{window = window}

	-- TODO renderer:draw
	local clear_color = {255, 255, 255, 255}
	while window:step() do
		renderer:clear{render_state = render_state, color = clear_color}
		renderer:draw_vertices{render_state = render_state, vertices = {
			{32,32,0,0, 255,0,0,255, 0,0},
			{32,96,0,0, 255,0,0,255, 0,0},
			{96,32,0,0, 255,0,0,255, 0,0},
		}}
		renderer:flush()
	end
end

main()
