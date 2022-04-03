local Engine = require("engine/engine")

local Fire = {}
Fire.WorldSys = Engine.World.Sys.new_metatable("fire")
Fire.GameSys = Engine.Game.Sys.new_metatable(Fire.WorldSys.sys_name)
Fire.GameSys.WorldSys = Fire.WorldSys
Fire.GameSys.WorldSys.State = {}
Fire.GameSys.WorldSys.State.defaults = {
	is_raining = false,
	turn_id = 0,

	fire_burn_length = 80,
	-- fire_burn_length = 15,  -- for testing
}
function Fire.GameSys.WorldSys:on_init()
	Engine.Core.Container.set_defaults(self.state, Fire.GameSys.WorldSys.State.defaults)

	self._entity = self.sim:require(Engine.Entity.WorldSys)
	self._template = self.sim:require(Engine.Template.WorldSys)
end
function Fire.GameSys.WorldSys:on_step()
	for _, entity in ipairs(self._entity:get_all_tagged_array(self.sys_name)) do
		local entity_id = self._entity:find_id(entity)

		if self.state.is_raining then
			self._template:instantiate("wood", {
				x = entity.x, y = entity.y, z = entity.z, tags = {wood_spawn = false},
			})
			self._template:instantiate("rock", {
				x = entity.x, y = entity.y, z = entity.z, tags = {rock_spawn = false},
			})
			self._entity:destroy(entity_id)
		elseif self.state.turn_id - (entity.turn_id or 0) >= self.state.fire_burn_length then
			self._template:instantiate("rock", {
				x = entity.x, y = entity.y, z = entity.z, tags = {rock_spawn = false},
			})
			self._entity:destroy(entity_id)
		end
	end
end
function Fire.GameSys.WorldSys:on_set_is_raining(is_raining)
	self.state.is_raining = is_raining
end
function Fire.GameSys.WorldSys:on_player_end_turn(turn_id)
	self.state.turn_id = turn_id
end

return Fire
