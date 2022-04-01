local Engine = require("engine/engine")

local Solid = {}
Solid.WorldSys = Engine.World.Sys.new_metatable("solid")
function Solid.WorldSys:on_init()
	self._entity = self.sim:require(Engine.Entity.WorldSys)
	self._entity:tag_bounds_index_add({"solid"})
end

Solid.GameSys = Engine.Game.Sys.new_metatable("solid")
Solid.GameSys.WorldSys = Solid.WorldSys

return Solid
