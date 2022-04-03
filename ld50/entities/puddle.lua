local Engine = require("engine/engine")

local Puddle = {}
Puddle.WorldSys = Engine.World.Sys.new_metatable("puddle")
Puddle.GameSys = Engine.Game.Sys.new_metatable(Puddle.WorldSys.sys_name)
Puddle.GameSys.WorldSys = Puddle.WorldSys
local fresh_water_tag = "fresh_water"
function Puddle.GameSys.WorldSys:on_entity_tag(entity_id, added_tags, entity)
	if Engine.Core.Container.array_try_find(added_tags, fresh_water_tag) ~= nil then
		local day_image_name = self.sys_name
		local night_image_name = day_image_name.."_night"
		local image_name = day_image_name

		if entity.image_name == entity.night_image_name then
			image_name = night_image_name
		end

		self.sim:get(Engine.Entity.WorldSys):set(entity_id, {
			image_name = image_name,
			day_image_name = day_image_name,
			night_image_name = night_image_name,
		})
	end
end
function Puddle.GameSys.WorldSys:on_entity_untag(entity_id, removed_tags, entity)
	if Engine.Core.Container.array_try_find(removed_tags, fresh_water_tag) ~= nil then
		local day_image_name = self.sys_name.."_empty"
		local night_image_name = day_image_name.."_night"
		local image_name = day_image_name

		if entity.image_name == entity.night_image_name then
			image_name = night_image_name
		end

		self.sim:get(Engine.Entity.WorldSys):set(entity_id, {
			image_name = image_name,
			day_image_name = day_image_name,
			night_image_name = night_image_name,
		})
	end
end

return Puddle
