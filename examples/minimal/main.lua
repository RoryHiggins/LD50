local Game = require("engine/engine/game")
local World = require("engine/engine/world")
local Client = require("engine/engine/client")
local Camera = require("engine/engine/camera")
local Controller = require("engine/engine/controller")

local width, height = 64, 64
local ExampleWorld = World.Sys.new_metatable("minimal_example")
function ExampleWorld:on_init()
	self._client_world = self.sim:require(Client.WorldSys)
	self._camera_world = self.sim:require(Camera.WorldSys)
	self._controller_world = self.sim:require(Controller.WorldSys)
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
	local vertex_array = self._client_world:get_vertex_array()
	vertex_array:add_triangle(0,0, 0,height, width,0, 255,0,0,255)
	vertex_array:add_triangle(width,0, 0,height, width,height, 0,255,0,255)
end

local state = {
	client = {width = width * 10, height = height * 10},
	world = {client = {width = width, height = height}},
}
local game = Game.Game.new(state)
game:require(Client.GameSys)
game:require(World.GameSys):require_world_sys(ExampleWorld)
game:require(Controller.GameSys)
game:run()
