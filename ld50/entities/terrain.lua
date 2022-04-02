local Engine = require("engine/engine")

local Terrain = {}
Terrain.WorldSys = Engine.World.Sys.new_metatable("terrain")
function Terrain.WorldSys:on_init()
	self._entity = self.sim:require(Engine.Entity.WorldSys)
end
function Terrain.WorldSys:on_start()
	local dirs = {
		{"n", 0, -8},
		{"s", 0, 8},
		{"w", -8, 0},
		{"e", 8, 0},
	}

	local tide_color = {57, 120, 168, 255}
	local night_tide_color = {57, 71, 120, 255}

	for entity_id, entity in pairs(self._entity:get_all_tagged(self.sys_name)) do
		if entity.dirt_color ~= nil then
			self._entity:add{
				x = entity.x, y = entity.y, z = entity.z - 1,
				width = entity.width, height = entity.height,
				image_name = "dirt_"..math.random(1, 8),
				tags = {animation = true},
				day_color = entity.dirt_color,
				night_color = entity.night_dirt_color or entity.dirt_color,
				r = entity.dirt_color[1],
				g = entity.dirt_color[2],
				b = entity.dirt_color[3],
				a = entity.dirt_color[4],
			}
		end

		local tide_anim_name = "tide_"
		for _, dir_tuple in ipairs(dirs) do
			local dir, x, y = dir_tuple[1], dir_tuple[2], dir_tuple[3]
			if self._entity:find_relative(entity_id, x, y, {"water"}, entity) ~= nil then
				tide_anim_name = tide_anim_name..dir
			end
		end
		if tide_anim_name ~= "tide_" then
			self._entity:add{
				x = entity.x, y = entity.y, z = entity.z - 1, width = entity.width, height = entity.height,
				anim_name = tide_anim_name,
				anim_speed = 0.02,
				tags = {animation = true},
				day_color = tide_color,
				night_color = night_tide_color,
				r = tide_color[1],
				g = tide_color[2],
				b = tide_color[3],
				a = tide_color[4],
			}
		end

		for _, dir_tuple in ipairs(dirs) do
			local dir, x, y = dir_tuple[1], dir_tuple[2], dir_tuple[3]
			local terrain_towards = self._entity:find_relative(
				entity_id, x, y, {"terrain"}, entity
			)
			if terrain_towards ~= nil then
				assert(not self._entity:find(terrain_towards).tags.water)
				self._entity:add{
					x = entity.x + x, y = entity.y + y, z = entity.z - 1,
					width = entity.width, height = entity.height,
					image_name = "terrain_mix_"..dir.."_"..math.random(1, 5),
					day_color = entity.terrain_color,
					night_color = entity.night_terrain_color or entity.terrain_color,
					r = entity.terrain_color[1],
					g = entity.terrain_color[2],
					b = entity.terrain_color[3],
					a = entity.terrain_color[4],
				}
			end
		end
	end
end


Terrain.GameSys = Engine.Game.Sys.new_metatable(Terrain.WorldSys.sys_name)
Terrain.GameSys.WorldSys = Terrain.WorldSys

return Terrain

