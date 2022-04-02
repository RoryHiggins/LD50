local Engine = require("engine/engine")

local Player = {}
Player.WorldSys = Engine.World.Sys.new_metatable("player")
function Player.WorldSys:on_init()
	local GameSystems = require("ld50/systems")
	self._data = self.sim:require(GameSystems.Data.WorldSys)
	self._entity = self.sim:require(Engine.Entity.WorldSys)
	self._template = self.sim:require(Engine.Template.WorldSys)
	self._camera_target = self.sim:require(Engine.CameraTarget.WorldSys)
	self._controller = self.sim:require(Engine.Controller.WorldSys)

	self._entity:tag_bounds_index_add({self.sys_name, "solid"})
	self._template:update("player", {
		tags = {solid = true},
	})
end
function Player.WorldSys:on_start()
	local entity_id = self._entity:find_tagged(self.sys_name)
	if entity_id == nil then
		entity_id = self._entity:add{
			tags = {player = true, solid = true},
			x = 8, y = 8, width = 8, height = 8,
			image_name = self.sys_name,
		}
	end

	self._camera_target:entity_set_default_camera(entity_id)
end
function Player.WorldSys:on_step()
	local repeat_step_count = 8
	local move_x, move_y = 0, 0
	if self._controller:get_pressed_repeating(1, "left", repeat_step_count) then
		move_x = move_x - 8
	end
	if self._controller:get_pressed_repeating(1, "right", repeat_step_count) then
		move_x = move_x + 8
	end
	if self._controller:get_pressed_repeating(1, "up", repeat_step_count) then
		move_y = move_y - 8
	end
	if self._controller:get_pressed_repeating(1, "down", repeat_step_count) then
		move_y = move_y + 8
	end

	for _, entity in ipairs(self._entity:get_all_tagged_array(self.sys_name)) do
		local entity_id = self._entity:find_id(entity)

		if self._entity:find_relative(entity_id, move_x, move_y, {"solid"}) == nil then
			self._entity:set_pos(
			entity_id,
			entity.x + move_x,
			entity.y + move_y
		)
		end
	end
end
function Player.WorldSys:on_draw()
end

Player.GameSys = Engine.Game.Sys.new_metatable(Player.WorldSys.sys_name)
Player.GameSys.WorldSys = Player.WorldSys

return Player
