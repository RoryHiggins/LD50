local Game = require("engine/engine/game")
local World = require("engine/engine/world")
local Client = require("engine/engine/client")
local Camera = require("engine/engine/camera")
local Controller = require("engine/engine/controller")

local AsciiFont = Client.wrappers.AsciiFont
local EntityIndex = Client.wrappers.EntityIndex

-- TODO move into GameSys.on_draw() once that is hooked up
local ExampleWorld = World.Sys.new_metatable("engine_test_example")
function ExampleWorld:on_init()
	self._client_world = self.sim:require(Client.WorldSys)
	self._camera_world = self.sim:require(Camera.WorldSys)
	self._controller_world = self.sim:require(Controller.WorldSys)

	-- TODO use Sprite.GameSys to allocate once hooked up
	self.sprites_u, self.sprites_v = self._client_world._context.texture_atlas:set_region_png_file{
		id = 0, filename = './examples/engine_test/data/sprites.png'}

	-- TODO use Text.GameSys to allocate and Text.WorldSys to draw once hooked up
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
function ExampleWorld:on_step()
	local camera = self._camera_world:get_default()
	local move_x, move_y = self._controller_world:get_dirs(self._controller_world.default_id)
	self._camera_world:set_pos(
		self._camera_world.default_name,
		math.floor(camera.transform.translate_x + move_x),
		math.floor(camera.transform.translate_y + move_y)
	)
end
function ExampleWorld:on_draw()
	local width, height = self._client_world:get_size()
	local vertex_array = self._client_world:get_vertex_array()
	vertex_array:add_triangle(0,0, 0,height, width,0, 255,0,0,255)
	vertex_array:add_triangle(width,0, 0,height, width,height, 0,255,0,255)
	vertex_array:add_line(0,0,width,height, 0,0,0,255, 0)
	vertex_array:add_line(0,0,0,height, 0,0,0,255, 0)
	vertex_array:add_line(0,0,width,0, 0,0,0,255, 0)
	vertex_array:add_line(0,0,8,0, 0,0,255,255, 0)
	vertex_array:add_line(0,0,0,8, 0,0,255,255, 0)
	vertex_array:add_line(7,0,7,8, 0,0,255,255, 0)
	vertex_array:add_line(0,7,8,7, 0,0,255,255, 0)

	vertex_array:add_point(24,8, 255,255,0,255, 1)
	self.entity_index:add_to_vertex_array{vertex_array = vertex_array}

	vertex_array:add_rect(16,16,48,48, 0,0,255,255, 0)
	self.ascii_font:add_text_to_vertex_array{
		vertex_array = vertex_array,
		str = "hello World!",
		x = 16, y = 16, max_w = 32, max_h = 32,
		color = {128,255,255,255}, depth = 0.0,
	}
	vertex_array:add_rect_outline(16,16,48,48, 255,255,0,255, 0)
end

local ExampleGame = Game.Sys.new_metatable("engine_test_example")
function ExampleGame:on_init()
	self._world_game = self.sim:require(World.GameSys)
	self._client_game = self.sim:require(Client.GameSys)
	self.sim:require(Controller.GameSys)

	self._world_game:require_world_sys(ExampleWorld)

	self._music = Client.wrappers.Music.new_file{
		filename = 'examples/engine_test/data/100ms_sine_440hz_22050hz_s16.ogg'
	}
	self._music:play{volume = 0.02, loop_forever = true}
end
function ExampleGame:on_draw()
	local width, height = self._client_game:get_size()
	local vertex_array = self._client_game:get_vertex_array()
	vertex_array:add_line(0,0,width,height, 255,255,0,255, 0)
	vertex_array:add_triangle(40,0, 0,40, 40,40, 255,255,0,255)
end


local function main()
	local state = {
		client = {width = 640, height = 640},
		world = {client = {width = 64, height = 64}},
	}
	local game_sim = Game.Game.new(state)
	game_sim:require(ExampleGame)
	game_sim:run()
end

main()
