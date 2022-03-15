-- luacheck: globals odClientWrapper

local window = odClientWrapper.Window.new{is_visible = true, width = 512, height = 512}
local atlas = odClientWrapper.TextureAtlas.new{window = window}
local renderer = odClientWrapper.Renderer.new{window = window}
local vertex_array = odClientWrapper.VertexArray.new{}
vertex_array:add_triangle(0,0, 0,512, 512,0, 255,0,0,255)
vertex_array:add_triangle(512,0, 0,512, 512,512, 0,255,0,255)

while window:step() do
	local draw_to_window = odClientWrapper.RenderState.new_ortho_2d{target = window, src = atlas}
	renderer:clear{render_state = draw_to_window, color = {255, 255, 255, 255}}
	renderer:draw_vertex_array{render_state = draw_to_window, vertex_array = vertex_array}
end
