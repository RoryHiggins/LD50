local Game = require("engine/engine/game")
local World = require("engine/engine/world")
local Client = require("engine/engine/client")

local AsciiFont = Client.wrappers.AsciiFont
local EntityIndex = Client.wrappers.EntityIndex

-- TODO move into GameSys.on_draw() once that is hooked up
local function world_draw_vertices(vertex_array, ascii_font, entity_index, width, height)
	vertex_array:add_triangle(0,0, 0,height, width,0, 255,0,0,255)
	vertex_array:add_triangle(width,0, 0,height, width,height, 0,255,0,255)
	vertex_array:add_line(0,0,width,height, 0,0,0,255, 0)
	vertex_array:add_line(0,0,0,height, 0,0,0,255, 0)
	vertex_array:add_line(0,0,width,0, 0,0,0,255, 0)
	vertex_array:add_line(0,0,8,0, 0,0,255,255, 0)
	vertex_array:add_line(0,0,0,8, 0,0,255,255, 0)
	vertex_array:add_line(7,0,7,8, 0,0,255,255, 0)
	vertex_array:add_line(0,7,8,7, 0,0,255,255, 0)

	vertex_array:add_rect(22,22,44,44, 0,0,255,255, 0)
	vertex_array:add_rect_outline(22,22,44,44, 255,255,0,255, 0)

	vertex_array:add_point(24,8, 255,255,0,255, 1)
	entity_index:add_to_vertex_array{vertex_array = vertex_array}

	ascii_font:add_text_to_vertex_array{
		vertex_array = vertex_array,
		str = "hello World!",
		x = 16, y = 16, max_w = 32, max_h = 32,
		color = {0,255,0,255}, depth = 0.0,
	}
end

-- TODO move into GameSys.on_draw() once that is hooked up
local function window_draw_vertices(vertex_array, width, height)
	vertex_array:add_line(0,0,width,height, 255,255,0,255, 0)
	vertex_array:add_triangle(40,0, 0,40, 40,40, 255,255,0,255)
end

local ExampleGameSys = Game.Sys.new_metatable("engine_test_example")
function ExampleGameSys:on_init()
	self._world_game = self.sim:require(World.GameSys)
	self._client = self.sim:require(Client.GameSys)

	self.sprites_u, self.sprites_v = self._client.context.texture_atlas:set_region_png_file{
		id = 0, filename = './examples/engine_test/data/sprites.png'}

	self.ascii_font = AsciiFont.new{
		u1 = self.sprites_u, v1 = self.sprites_v + 160,
		u2 = self.sprites_u + 64, v2 = self.sprites_v + 256,
		char_w = 8, char_h = 8, char_first = ' ', char_last = '~',
	}

	self.entity_index = EntityIndex.new{}
	for i = 1, 4 do
		self.entity_index:set_collider(
			i, (8 * i), 8, (8 * i) + 8, 16)
		self.entity_index:set_sprite(
			i, self.sprites_u + 16, self.sprites_v + 24, self.sprites_u + 24, self.sprites_v + 32)
	end
end
function ExampleGameSys:on_step()
	local context = self._client.context
	local client_world = self._world_game.world:get(Client.WorldSys)
	local target = client_world._render_target

	world_draw_vertices(
		client_world:get_vertex_array(), self.ascii_font, self.entity_index, target.settings.width, target.settings.height)
	window_draw_vertices(self._client:get_vertex_array(), context.settings.window.width, context.settings.window.height)

	context.renderer:clear{target = target.render_texture, color = {255, 255, 255, 255}}

	local offset = math.floor((context.step_count / 8) % 8)
	local offset_render_state = target.render_state_ortho_2d:copy():set_viewport_ortho_2d{
		viewport = {x = offset, y = offset, width = target.settings.width, height = target.settings.height},
		target = target.render_texture}
	context.renderer:draw_vertex_array{
		render_state = offset_render_state,
		src = context.texture_atlas,
		target = target.render_texture,
		vertex_array = client_world:get_vertex_array()}

	context.renderer:draw_texture{render_state = context.render_state_passthrough, src = target.render_texture}
	context.renderer:draw_vertex_array{
		render_state = context.render_state_ortho_2d,
		src = context.texture_atlas,
		vertex_array = self._client:get_vertex_array()}
end


local function main()
	local game_sim = Game.Game.new()
	game_sim:require(ExampleGameSys)
	game_sim:run()
end

main()
