local Engine = require("engine/engine")

local Data = {}
Data.WorldSys = Engine.World.Sys.new_metatable("data")
function Data.WorldSys:on_init()
	self._image = self.sim:require(Engine.Image.WorldSys)
	self._animation = self.sim:require(Engine.Animation.WorldSys)

	local filename = "./examples/engine_test/data/sprites.png"
	local file_type = "png"
	local grid_size = 8

	-- local images = {
	-- 	solid = {0, 24},
	-- }
	-- self._image:set_batch(images, filename, file_type, grid_size)
	self._animation:set_strip("player", 16,0, 72,8, filename, file_type, grid_size)
end

Data.GameSys = Engine.Game.Sys.new_metatable("data")
Data.GameSys.WorldSys = Data.WorldSys

return Data
