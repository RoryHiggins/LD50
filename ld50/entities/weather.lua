local Engine = require("engine/engine")

local Weather = {}
Weather.WorldSys = Engine.World.Sys.new_metatable("weather")
Weather.GameSys = Engine.Game.Sys.new_metatable(Weather.WorldSys.sys_name)
Weather.GameSys.WorldSys = Weather.WorldSys
Weather.GameSys.WorldSys.State = {}
Weather.GameSys.WorldSys.State.defaults = {
	is_raining = false,
	is_nighttime = false,

	rain_start = 150,
	rain_length = 75,
	rain_wait_length = 425,

	-- alts for rain testing
	-- rain_start = 15,
	-- rain_length = 15,
	-- rain_wait_length = 42,
}
function Weather.GameSys.WorldSys:on_init()
	Engine.Core.Container.set_defaults(self.state, Weather.GameSys.WorldSys.State.defaults)

	self._client = self.sim:require(Engine.Client.WorldSys)
	self._image = self.sim:require(Engine.Image.WorldSys)
	self._entity = self.sim:require(Engine.Entity.WorldSys)
end
function Weather.GameSys.WorldSys:restore_resources()
	local spawn_tags = {"wood", "rock", "fresh_water", "food"}
	local spawn_tag_pairs = {}
	for _, tag in ipairs(spawn_tags) do
		spawn_tag_pairs[#spawn_tag_pairs + 1] = {tag, tag.."_spawn"}
	end

	for entity_id, entity in ipairs(self._entity:get_all_raw()) do
		local tags = entity.tags
		if tags ~= nil then
			for _, spawn_tag_pair in ipairs(spawn_tag_pairs) do
				local tag, spawn_tag = spawn_tag_pair[1], spawn_tag_pair[2]

				if tags[spawn_tag] == true and tags[tag] ~= true then
					self._entity:set(entity_id, {a = 255, tags = {[tag] = true}}, entity)
				end
			end
		end
	end
end
function Weather.GameSys.WorldSys:on_player_end_turn(turn_id)
	local should_rain = turn_id >= self.state.rain_start and turn_id < (self.state.rain_start + self.state.rain_length)
	if self.state.is_raining ~= should_rain then
		if not should_rain then  -- end of rain
			self:restore_resources()
			self.state.rain_start = turn_id + self.state.rain_wait_length
		end

		self.state.is_raining = should_rain
		self.sim:broadcast("on_set_is_raining", should_rain)
	end
end
function Weather.GameSys.WorldSys:on_draw()
	if self.state.is_raining then
		local start_x, start_y = self._client:camera_get_pos("default")
		local width, height = self._client:camera_get_size("default")
		start_x, start_y = math.floor(start_x / 8) * 8, math.floor(start_y / 8) * 8

		local rain_r, rain_g, rain_b, rain_a = 57, 71, 120, 255
		if self.state.is_nighttime then
			rain_r, rain_g, rain_b, rain_a = 57, 49, 75, 255
		end

		local image_world = self._image
		for x = start_x, start_x + width, 8 do
			for y = start_y, start_y + height, 8 do
				local rain_x = x + math.random(-3, 3)
				local rain_y = y + math.random(-3, 3)

				image_world:draw("rain", rain_x, rain_y, 8, 8, rain_r, rain_g, rain_b, rain_a, -98)
			end
		end
	end
end
function Weather.GameSys.WorldSys:on_set_is_nighttime(is_nighttime)
	self.state.is_nighttime = is_nighttime
end

return Weather
