local Window = require("engine/client/window")
local TextureAtlas = require("engine/client/texture_atlas")
local RenderTexture = require("engine/client/render_texture")
local Renderer = require("engine/client/renderer")
local RenderState = require("engine/client/render_state")
local AsciiFont = require("engine/client/ascii_font")
local VertexArray = require("engine/client/vertex_array")
local EntityIndex = require("engine/client/entity_index")

local function main()
	local window = Window.new{width = 512, height = 512}

	local atlas = TextureAtlas.new{window = window}
	local sprites_u, sprites_v = atlas:set_region_png_file{id = 0, filename = './examples/engine_test/data/sprites.png'}

	-- add some entities
	local entity_index = EntityIndex.new{}
	for i = 1, 4 do
		entity_index:set_collider(i, (8 * i), 8, (8 * i) + 8, 16)
		entity_index:set_sprite(i, sprites_u + 16, sprites_v + 24, sprites_u + 24, sprites_v + 32)
	end

	local ascii_font = AsciiFont.new{
		u1 = sprites_u, v1 = sprites_v + 160,
		u2 = sprites_u + 64, v2 = sprites_v + 256,
		char_w = 8, char_h = 8,
		char_first = ' ', char_last = '~',
	}

	local vertex_array = VertexArray.new{}
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

	local window_vertex_array = VertexArray.new{}
	window_vertex_array:add_line(0,0,512,512, 255,255,0,255, 0)
	window_vertex_array:add_triangle(40,0, 0,40, 40,40, 255,255,0,255)

	local renderer = Renderer.new{window = window}
	local game_render_texture = RenderTexture.new{window = window, width = 128, height = 128}

	local frame = 0
	while window:step() do
		frame = frame + 1

		local draw_to_game = RenderState.new_ortho_2d{target = game_render_texture}
		renderer:clear{render_state = draw_to_game, target = game_render_texture, color = {255, 255, 255, 255}}
		draw_to_game:set_viewport_ortho_2d{
			viewport = {x = math.floor((frame / 8) % 8), y = math.floor((frame / 8) % 8), width = 128, height = 128},
			target = game_render_texture
		}
		renderer:draw_vertex_array{
			render_state = draw_to_game, src = atlas, target = game_render_texture, vertex_array = vertex_array}

		local draw_to_window = RenderState.new_ortho_2d{target = window}
		renderer:clear{render_state = draw_to_window, src = atlas, color = {255, 255, 255, 255}}

		local copy_game_to_window = RenderState.new{target = window}
		renderer:draw_texture{render_state = copy_game_to_window, src = game_render_texture}
		renderer:draw_vertex_array{
			render_state = draw_to_window, src = atlas, vertex_array = window_vertex_array}
	end
end

main()
