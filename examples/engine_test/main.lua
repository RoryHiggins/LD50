local Game = require("engine/engine/game")
local World = require("engine/engine/world")
local Client = require("engine/engine/client")
local Camera = require("engine/engine/camera")
local Controller = require("engine/engine/controller")
local Entity = require("engine/engine/entity")
local Image = require("engine/engine/image")
local Text = require("engine/engine/text")

local ExampleWorld = World.Sys.new_metatable("engine_test_example")
function ExampleWorld:on_init()
	self._client_world = self.sim:require(Client.WorldSys)
	self._camera_world = self.sim:require(Camera.WorldSys)
	self._controller_world = self.sim:require(Controller.WorldSys)
	self._entity_world = self.sim:require(Entity.WorldSys)
	self._image_world = self.sim:require(Image.WorldSys)
	self._text_world = self.sim:require(Text.WorldSys)

	local images = {
		blank = {0, 0},
		none = {8, 0},
		wall = {16, 24},
	}

	self._image_world:set_batch(images, "./examples/engine_test/data/sprites.png", "png", 8)
end
function ExampleWorld:on_start()
	for i = 1, 4 do
		self._entity_world:add{
			x = (8 * i), y = 8, width = 8, height = 8,
			image_name = "wall",
			z = -1,
		}
	end
end
function ExampleWorld:on_step()
	local camera = self._camera_world:get_default()
	local move_x, move_y = self._controller_world:get_dirs(self._controller_world.default_id)
	self._camera_world:set_pos(
		self._camera_world.default_camera_name,
		math.floor(-camera.transform.translate_x - move_x),
		math.floor(-camera.transform.translate_y - move_y)
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

	vertex_array:add_rect(16,16,48,48, 0,0,255,255, 0)
	self._text_world:draw("default", "hello world!", 16,16, 32,32, 128,255,255,255)
	vertex_array:add_rect_outline(16,16,48,48, 255,255,0,255, 0)
end

local ExampleGame = Game.Sys.new_metatable("engine_test_example")
function ExampleGame:on_init()
	self._world_game = self.sim:require(World.GameSys)
	self._client_game = self.sim:require(Client.GameSys)
	self._image_game = self.sim:require(Image.GameSys)
	self.sim:require(Controller.GameSys)

	self._world_game:require_world_sys(ExampleWorld)

	self._music = Client.Wrappers.Music.new_file{
		filename = './ld50/data/sea_ambient.ogg'
	}
	self._music:play{volume = 0.5, loop_forever = true}
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
