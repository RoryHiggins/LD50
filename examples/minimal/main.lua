local Game = require("engine/engine/game")
local World = require("engine/engine/world")
local Client = require("engine/engine/client")
local Camera = require("engine/engine/camera")

local ExampleWorld = World.Sys.new_metatable("minimal_example")
function ExampleWorld:on_init()
	self._client_world = self.sim:require(Client.WorldSys)
	self._camera_world = self.sim:require(Camera.WorldSys)
end
function ExampleWorld:on_step()
	local mouse_x, mouse_y = self._client_world:_get_mouse_pos()
	local width, height = self._client_world:get_size()
	self._camera_world:set_pos(
		self._camera_world.default_name,
		math.floor(mouse_x - (width / 2)),
		math.floor(mouse_y - (height / 2))
	)
end
function ExampleWorld:on_draw()
	local vertex_array = self._client_world:get_vertex_array()
	vertex_array:add_triangle(0,0, 0,64, 64,0, 255,0,0,255)
	vertex_array:add_triangle(64,0, 0,64, 64,64, 0,255,0,255)
end

local state = {
	client = {width = 640, height = 640},
	world = {client = {width = 64, height = 64}},
}
local game = Game.Game.new(state)
game:require(Client.GameSys)
game:require(World.GameSys):require_world_sys(ExampleWorld)
game:run()
