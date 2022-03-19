local Window = require("engine/client/window")
local TextureAtlas = require("engine/client/texture_atlas")
local Renderer = require("engine/client/renderer")
local RenderState = require("engine/client/render_state")
local VertexArray = require("engine/client/vertex_array")

local window = Window.new{width = 512, height = 512}
local atlas = TextureAtlas.new{window = window}
local renderer = Renderer.new{window = window}
local vertex_array = VertexArray.new{}
vertex_array:add_triangle(0,0, 0,512, 512,0, 255,0,0,255)
vertex_array:add_triangle(512,0, 0,512, 512,512, 0,255,0,255)

while window:step() do
	local draw_to_window = RenderState.new_ortho_2d{target = window}

	local mouse = window:get_mouse_state()
	draw_to_window:transform_view{translate_x = mouse.x - 256, translate_y = mouse.y - 256}

	renderer:clear{render_state = draw_to_window, color = {255, 255, 255, 255}}
	renderer:draw_vertex_array{render_state = draw_to_window, src = atlas, vertex_array = vertex_array}
end
