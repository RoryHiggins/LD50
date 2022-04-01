local Engine = require("engine/engine")

local Player = {}
Player.WorldSys = Engine.World.Sys.new_metatable("player")
function Player.WorldSys:on_init()
	self._entity = self.sim:require(Engine.Entity.WorldSys)
	self._camera_target = self.sim:require(Engine.CameraTarget.WorldSys)
	self._controller = self.sim:require(Engine.Controller.WorldSys)
end
function Player.WorldSys:on_start()
	local entity_id = self._entity:find_tagged(self.sys_name)
	if entity_id == nil then
		--[[entity_id = --]]self._entity:add{
			tags = {animation = true, player = true},
			x = 8, y = 8, width = 8, height = 8,
			anim_name = self.sys_name, anim_speed = 0.2,
		}
	end

	-- self._camera_target:entity_set_default_camera(entity_id)
end
function Player.WorldSys:on_step()
	local move_x, move_y = self._controller:get_dirs(self._controller.default_id)

	for _, entity in ipairs(self._entity:get_all_tagged_raw(self.sys_name)) do
		local entity_id = self._entity:find_id(entity)

		self._entity:set_pos(
			entity_id,
			entity.x + move_x,
			entity.y + move_y
		)
	end
end

Player.GameSys = Engine.Game.Sys.new_metatable("player")
Player.GameSys.WorldSys = Player.WorldSys

return Player
