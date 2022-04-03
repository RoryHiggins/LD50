local Engine = require("engine/engine")

local Math = Engine.Core.Math

local level_threshold = 100
local level_thresholds = 4
local level_threshold_min = 0
local level_threshold_low = level_threshold
local level_threshold_mid = math.floor((level_thresholds / 2) * level_threshold)
local level_threshold_high = (level_thresholds - 1) * level_threshold
local level_threshold_max = (level_thresholds * level_threshold) - 1
local level_threshold_danger_low = level_threshold_low - math.floor(level_threshold / 2)
local level_threshold_danger_high = level_threshold_high + math.floor(level_threshold / 2)

local Player = {}
Player.WorldSys = Engine.World.Sys.new_metatable("player")
Player.WorldSys.State = {}
Player.WorldSys.State.defaults = {
	-- status
	health = level_threshold_max,
	hunger = level_threshold_min,
	thirst = level_threshold_low,
	temperature = level_threshold_low,
	is_resting = false,
	dead = false,

	-- resources
	wood = 0,
	rock = 0,

	-- wood = 20,  -- for testing
	-- rock = 20,

	-- progression
	turn_id = 0,
	is_nighttime = false,
	is_raining = false,

	-- dialog
	dialog_stack = {},
	dialog_choice = 1,
	eat_done = false,
	drink_done = false,
	tree_rest_done = false,
	make_fire_done = false,

	-- configuration
	day_clear_color = {57, 120, 168, 255},
	night_clear_color = {57, 49, 75, 255},
	turns_per_day = level_threshold,  -- makes the math so much easier...
	fraction_of_day_is_nighttime = 0.4,
	prompt_toggle_steps = 30,
}
-- rendering
function Player.WorldSys:draw_dialog_ui()
	if #self.state.dialog_stack == 0 then
		local x, y = self._client:camera_get_pos("default")
		local z = -100
		local width, height = self._client:camera_get_size("default")

		local text = "action:z"
		self._text:draw(
			self._text.font_default_name, text, x + width - 64, y + height - 8, nil, nil,
			255, 255, 255, 255, z)
		return
	end

	local dialog = self.state.dialog_stack[#self.state.dialog_stack]
	local choices = dialog.choices

	local x, y = self._client:camera_get_pos("default")
	local z = -100
	local width, height = self._client:camera_get_size("default")

	local border_r, border_g, border_b, border_a, border_z = 160, 91, 83, 255, z
	local ui_r, ui_g, ui_b, ui_a, ui_z = 191, 121, 88, 255, z

	local dialog_r, dialog_g, dialog_b, dialog_a = 255, 255, 255, 255
	local dialog_width = width
	local dialog_height = 24 + (8 * #choices)
	local dialog_x = x
	local dialog_y = y + height - dialog_height
	local dialog_z = ui_z - 1

	local vertex_array = self._client:get_vertex_array()
	vertex_array:add_line(
		dialog_x, dialog_y - 1, dialog_x + dialog_width, dialog_y - 1,
		border_r, border_g, border_b, border_a, border_z)
	vertex_array:add_rect(
		dialog_x, dialog_y, dialog_x + dialog_width, dialog_y + dialog_height,
		ui_r, ui_g, ui_b, ui_a, ui_z)

	self._text:draw(
		self._text.font_default_name, dialog.text, dialog_x, dialog_y, dialog_width,
		24, dialog_r, dialog_g, dialog_b, dialog_a, dialog_z)

	for i, choice in ipairs(choices) do
		local text = choice.text
		if self.state.dialog_choice == i then
			text = ">"..text
		else
			text = "-"..text
		end

		self._text:draw(
			self._text.font_default_name, text, dialog_x, dialog_y + 24 + (8 * (i - 1)), dialog_width,
			8, dialog_r, dialog_g, dialog_b, dialog_a, dialog_z)
	end

	if self.sim.step_id % self.state.prompt_toggle_steps < (self.state.prompt_toggle_steps / 2) then
		local ellipsis_x = dialog_x + dialog_width - 8
		local ellipsis_y = dialog_y + dialog_height - 8
		vertex_array:add_rect(
			ellipsis_x, ellipsis_y, ellipsis_x + 8, ellipsis_y + 8,
			ui_r, ui_g, ui_b, ui_a, ui_z)
		self._image:draw(
			"prompt", ellipsis_x, ellipsis_y, 8, 8, dialog_r, dialog_g, dialog_b, dialog_a, dialog_z)
		end
end
function Player.WorldSys:draw_fog()
	local _, entity = self._entity:find_tagged(self.sys_name)
	local player_x, player_y = entity.x or 0, entity.y or 0

	local start_x, start_y = self._client:camera_get_pos("default")
	local width, height = self._client:camera_get_size("default")
	start_x, start_y = math.floor(start_x / 8) * 8, math.floor(start_y / 8) * 8

	local light_tags = {"light"}
	local dim_dist = 8
	local sight_night = 40
	local dim_sight_night = 20
	local blackout_half_image = "blackout_half_"..(1 + (math.floor(self.sim.step_id / 10) % 2))
	local image_world = self._image

	if self.state.is_nighttime then
		local r,g,b,a = 48,44,46,255

		for x = start_x, start_x + width, 8 do
			for y = start_y, start_y + height, 8 do
				if Math.distance(x, y, player_x, player_y) >= sight_night then
					image_world:draw("blackout_full", x, y, 8, 8, r,g,b,a, -99)
				elseif self._entity:find_in(x + 4 - dim_dist, y + 4 - dim_dist, dim_dist * 2, dim_dist * 2, light_tags) == nil
				and Math.distance(x, y, player_x, player_y) >= dim_sight_night then
					image_world:draw(blackout_half_image, x, y, 8, 8, r,g,b,a, -99)
				end
			end
		end
	end
end
function Player.WorldSys:draw_status_ui()
	local x, y = self._client:camera_get_pos("default")
	local z = -100
	local width, _ = self._client:camera_get_size("default")

	local border_r, border_g, border_b, border_a, border_z = 160, 91, 83, 255, z
	local ui_r, ui_g, ui_b, ui_a, ui_z = 191, 121, 88, 255, z

	local status_r, status_g, status_b, status_a = 255, 255, 255, 255
	local status_width = 32
	local status_height = 24
	local status_x = x + width - status_width
	local status_y = y
	local status_z = ui_z - 2

	local vertex_array = self._client:get_vertex_array()
	vertex_array:add_line(
		status_x - 1, status_y, status_x - 1, status_y + status_height,
		border_r, border_g, border_b, border_a, border_z)
	vertex_array:add_line(
		status_x - 1, status_y + status_height, status_x + status_width, status_y + status_height,
		border_r, border_g, border_b, border_a, border_z)
	vertex_array:add_rect(
		status_x, status_y, status_x + status_width, status_y + status_height,
		ui_r, ui_g, ui_b, ui_a, ui_z)

	self._image:draw(
		"health_"..self:get_attr_level(self.state.health), status_x, status_y, 8, 8, 255,255,255,255, status_z)
	self._image:draw(
		"temperature_"..self:get_attr_level(self.state.temperature), status_x + 8, status_y, 8, 8, 255,255,255,255, status_z)
	self._image:draw(
		"thirst_"..self:get_attr_level(self.state.thirst), status_x + 16, status_y, 8, 8, 255,255,255,255, status_z)
	self._image:draw(
		"hunger_"..self:get_attr_level(self.state.hunger), status_x + 24, status_y, 8, 8, 255,255,255,255, status_z)

	if self.sim.step_id % self.state.prompt_toggle_steps < (self.state.prompt_toggle_steps / 2) then
		local health_loss = false
		if self.state.temperature <= level_threshold_danger_low
		or self.state.temperature >= level_threshold_danger_high then
			health_loss = true
			vertex_array:add_rect(
				status_x + 8, status_y, status_x + 16, status_y + 8, 255,0,0,255, ui_z - 1)
		end
		if self.state.thirst >= level_threshold_danger_high then
			health_loss = true
			vertex_array:add_rect(
				status_x + 16, status_y, status_x + 24, status_y + 8, 255,0,0,255, ui_z - 1)
		end
		if self.state.hunger >= level_threshold_danger_high then
			health_loss = true
			vertex_array:add_rect(
				status_x + 24, status_y, status_x + 32, status_y + 8, 255,0,0,255, ui_z - 1)
		end
		if health_loss then
			vertex_array:add_rect(
				status_x, status_y, status_x + 8, status_y + 8, 255,0,0,255, ui_z - 1)
		end
	end

	local time = "day"
	if self.state.is_nighttime then
		time = "night"
	elseif self:is_evening_at_turn(self.state.turn_id) then
		time = "evening"
	end
	self._image:draw(
		"time_"..time, status_x + status_width - 8, status_y + 8, 8, 8, 255,255,255,255, status_z)

	self._image:draw(
		"wood", status_x, status_y + 8, 8, 8, 255,255,255,255, status_z)
	self._text:draw(
		self._text.font_default_name, tostring(self.state.wood), status_x + 8, status_y + 8, status_width,
		status_height, status_r, status_g, status_b, status_a, status_z)

	self._image:draw(
		"rock", status_x, status_y + 16, 8, 8, 255,255,255,255, status_z)
	self._text:draw(
		self._text.font_default_name, tostring(self.state.rock), status_x + 8, status_y + 16, status_width,
		status_height, status_r, status_g, status_b, status_a, status_z)
end

-- dialog actions
function Player.WorldSys:push_dialog(text, choices)
	assert(type(text) == "string")
	assert(type(choices) == "table" or choices == nil)

	self.state.dialog_choice = 1
	table.insert(self.state.dialog_stack, {text = text, choices = choices or {}})
end
function Player.WorldSys:pop_dialog()
	if #self.state.dialog_stack == 0 then
		return true
	end

	self.state.dialog_stack[#self.state.dialog_stack] = nil
	self.state.dialog_choice = 1
end
function Player.WorldSys:clear_dialog()
	while #self.state.dialog_stack > 0 do
		self:pop_dialog()
	end
end
function Player.WorldSys:perform_dialog_action(action, args)
	assert(type(action) == "string")

	if self[action] then
		self[action](self, Engine.Core.Shim.unpack(args or {}))
	else
		Engine.Core.Logging.warning("Unknown action=%s", action)
		self:clear_dialog()
		self:push_dialog("i cant do that right now")
	end
end
function Player.WorldSys:noop_dialog()
end
--[[function Player.WorldSys:dont_want_that_dialog()
	self:clear_dialog()

	local refusals = {
		"i dont want that",
		"how about no",
		"what am i even thinking",
	}
	local refusal = refusals[math.random(1, #refusals)]
	self:push_dialog(refusal)
end--]]
function Player.WorldSys:step_dialog()
	if #self.state.dialog_stack == 0 then
		return true
	end

	local dialog = self.state.dialog_stack[#self.state.dialog_stack]
	assert(dialog.text ~= nil)
	assert(dialog.choices ~= nil)

	if self._controller:get_pressed(1, "up") and #dialog.choices > 0 then
		self.state.dialog_choice = self.state.dialog_choice - 1
		if self.state.dialog_choice < 1 then
			self.state.dialog_choice = #dialog.choices
		end
		self.audio:play{}
		return false
	end
	if self._controller:get_pressed(1, "down") and #dialog.choices > 0 then
		self.state.dialog_choice = self.state.dialog_choice + 1
		if self.state.dialog_choice > #dialog.choices then
			self.state.dialog_choice = 1
		end
		self.audio:play{}
		return false
	end

	self.state.dialog_choice = Math.clamp(self.state.dialog_choice, 1, #dialog.choices)

	if self._controller:get_pressed(1, "a") then
		if #dialog.choices == 0 then
			self:pop_dialog()
		else
			self:perform_dialog_action(
				dialog.choices[self.state.dialog_choice].action,
				dialog.choices[self.state.dialog_choice].args)
		end
		self.audio:play{}
		return false
	end

	if self._controller:get_pressed(1, "b") and not self.state.dead then
		self:pop_dialog()
		self.audio:play{}
		return
	end

	return false
end

-- game actions
function Player.WorldSys:restart()
	self.sim:stop()
end
function Player.WorldSys:quit()
	self.sim._game:stop()
end

-- attribute actions
function Player.WorldSys:get_attr_level(attribute_value)
	return math.floor(attribute_value / level_threshold) + 1
end
function Player.WorldSys:clamped_attr_add(attribute_value, amount)
	return Math.clamp(
		attribute_value + amount, 0, level_threshold * level_thresholds - 1
	)
end
function Player.WorldSys:offset_hunger(amount)
	local hunger_after = self:clamped_attr_add(self.state.hunger, amount)
	if self.state.hunger < level_threshold_high and hunger_after >= level_threshold_high then
		if not self.state.eat_done then
			self:push_dialog("maybe those trees will have fruit")
		end
		self:push_dialog("starving...")
	end
	self.state.hunger = hunger_after
end
function Player.WorldSys:offset_thirst(amount)
	local thirst_after = self:clamped_attr_add(self.state.thirst, amount)
	if self.state.thirst < level_threshold_high and thirst_after >= level_threshold_high then
		if not self.state.drink_done then
			self:push_dialog("must be fresh water somewhere")
		end
		self:push_dialog("so thirsty")
	end
	self.state.thirst = thirst_after
end
function Player.WorldSys:offset_temperature(amount)
	local temperature_after = self:clamped_attr_add(self.state.temperature, amount)
	if self.state.temperature < level_threshold_high and temperature_after >= level_threshold_high then
		if not self.state.tree_rest_done then
			self:push_dialog("maybe i can find water or a tree with some shade")
		end

		self:push_dialog("too hot... need to cool down")
	end
	if self.state.temperature >= level_threshold_low and temperature_after < level_threshold_low then
		if not self.state.make_fire_done then
			self:push_dialog("maybe i can make a fire")
		end
		self:push_dialog("its freezing")
	end
	self.state.temperature = temperature_after
end
function Player.WorldSys:offset_health(amount)
	local health_after = self:clamped_attr_add(self.state.health, amount)

	if health_after == 0 then
		self.state.dead = true
		self:clear_dialog()
		self:push_dialog("you passed away.  try again?", {
			{action = "restart", text = "try again"},
			{action = "quit", text = "quit"},
		})
		self:push_dialog("so this is how it ends")
	elseif self.state.health >= level_threshold_low and health_after < level_threshold_low then
		self:push_dialog("i cant much more take this")
	end

	self.state.health = health_after
end

-- resource actions
function Player.WorldSys:wood_take(entity_id)
	if entity_id == nil then
		self:push_dialog("what wood?")
		return
	end

	self._entity:set(entity_id, {a = 0, tags = {wood = false}})
	self.state.wood = self.state.wood + 1

	self:clear_dialog()
	self:end_turn()
end
function Player.WorldSys:rock_take(entity_id)
	if entity_id == nil then
		self:push_dialog("what rock?")
		return
	end

	self._entity:set(entity_id, {a = 0, tags = {rock = false}})
	self.state.rock = self.state.rock + 1
	self:clear_dialog()
	self:end_turn()
end
function Player.WorldSys:puddle_drink(entity_id)
	if entity_id == nil then
		self:push_dialog("what puddle?")
		return
	end

	self._entity:set(entity_id, {tags = {fresh_water = false}})
	self:offset_thirst(-level_threshold)

	local hot = self.state.temperature > level_threshold_mid
	if hot then
		self:offset_temperature(-100)
	end

	self:clear_dialog()

	self.state.drink_done = true
	self:push_dialog("better than the tap water at home")
	self:end_turn(5)
end
function Player.WorldSys:tree_eat_fruit(entity_id)
	if entity_id == nil then
		self:push_dialog("what fruit?")
		return
	end

	self._entity:set(entity_id, {tags = {food = false}})
	self:offset_thirst(-level_threshold)
	self:clear_dialog()

	self.state.drink_done = true
	self:push_dialog("disgusting, but im still alive")
	self:end_turn(5)
end

-- build actions
function Player.WorldSys:fire_make()
	local _, entity = self._entity:find_tagged(self.sys_name)
	local x, y, z = entity.x or 0, entity.y or 0, (entity.z + 1) or -2

	if self.state.wood < 1 then
		self:push_dialog("need to find more wood first")
		return
	end
	if self.state.rock < 1 then
		self:push_dialog("should find another sturdy trusty rock")
		return
	end
	if self._entity:find_in(x, y, 8, 8, {"fire"}) ~= nil then
		self:push_dialog("this fire has enough tinder already")
		return
	end

	self._template:instantiate("fire", {x = x, y = y, z = z, turn_id = self.state.turn_id})
	self.state.wood = self.state.wood - 1
	self.state.rock = self.state.rock - 1

	self:clear_dialog()
	self:push_dialog("this fire will have to do")
	self:end_turn(5)
end
function Player.WorldSys:shelter_make()
	local _, entity = self._entity:find_tagged(self.sys_name)
	local x, y, z = entity.x or 0, entity.y or 0, (entity.z + 1) or -2

	if self.state.wood < 8 then
		self:push_dialog("need to find more wood first")
		return
	end
	if self.state.rock < 8 then
		self:push_dialog("should find another sturdy trusty rock")
		return
	end
	if self._entity:find_in(x, y, 8, 8, {"shelter"}) ~= nil then
		self:push_dialog("this shelter is already sturdy enough")
		return
	end

	self._template:instantiate("shelter", {x = x, y = y, z = z, turn_id = self.state.turn_id})
	self.state.wood = self.state.wood - 8
	self.state.rock = self.state.rock - 8

	self:clear_dialog()
	self:push_dialog("this shelter looks sturdy enough")
	self:end_turn(10)
end
function Player.WorldSys:shelter_scrap(entity_id)
	if entity_id == nil then
		self:push_dialog("what shelter?")
		return
	end

	self._entity:destroy(entity_id)

	self.state.wood = self.state.wood + 8
	self.state.rock = self.state.rock + 8
	self:clear_dialog()
	self:end_turn(5)
end
function Player.WorldSys:fire_scrap(entity_id)
	if entity_id == nil then
		self:push_dialog("what fire?")
		return
	end

	self._entity:destroy(entity_id)

	self.state.wood = self.state.wood + 1
	self.state.rock = self.state.rock + 1
	self:clear_dialog()
	self:end_turn(2)
end

-- recovery actions
function Player.WorldSys:rest()
	if not self:is_in_shelter() then
		self:push_dialog("i need somewhere sheltered to rest")
		return
	end

	if not self:is_nighttime_at_turn(self.state.turn_id) then
		self:push_dialog("too bright for any rest, lets wait for night")
		return
	end

	self:clear_dialog()
	while self.state.is_nighttime do
		self:end_turn()
	end
end
function Player.WorldSys:cool_off()
	if not self:is_in_shade() then
		self:push_dialog("i need somewhere in the shade to cool off")
		return
	end

	if self:is_nighttime_at_turn(self.state.turn_id) then
		self:push_dialog("its already cold enough outside")
		return
	end

	self:clear_dialog()
	while not self:is_nighttime_at_turn(self.state.turn_id) and (self.state.temperature >= level_threshold_mid + 50) do
		self:end_turn()
	end
end
function Player.WorldSys:warm_up()
	if not self:is_in_shelter() then
		self:push_dialog("i need somewhere sheltered to warm up")
		return
	end

	if not self:is_nighttime_at_turn(self.state.turn_id) then
		self:push_dialog("its already warm enough outside")
		return
	end

	self:clear_dialog()
	while self:is_nighttime_at_turn(self.state.turn_id) and (self.state.temperature <= level_threshold_mid - 50) do
		self:end_turn()
	end
end

-- action menus
function Player.WorldSys:push_interact_dialogue()
	local choices = {}
	local wood_near = self:find_in_range{"wood"}
	local rock_near = self:find_in_range{"rock"}
	local puddle_fresh_water_near = self:find_in_range{"puddle", "fresh_water"}
	local tree_fruit_near = self:find_in_range{"tree", "food"}
	local tree_near = self:find_in_range{"tree"}
	local shelter_near = self:find_in_range{"shelter"}
	local fire_near = self:find_in_range{"fire"}

	local hot = self.state.temperature >= level_threshold_high
	local cold = self.state.temperature <= level_threshold_low
	local night = self:is_nighttime_at_turn(self.state.turn_id)
	local evening = self:is_evening_at_turn(self.state.turn_id)
	local day = not night and not evening

	if wood_near ~= nil then
		choices[#choices + 1] = {action = "wood_take", text = "wood:take", args = {wood_near}}
	end
	if rock_near ~= nil then
		choices[#choices + 1] = {action = "rock_take", text = "rock:take", args = {rock_near}}
	end
	if puddle_fresh_water_near ~= nil then
		choices[#choices + 1] = {action = "puddle_drink", text = "puddle:drink", args = {puddle_fresh_water_near}}
	end
	if tree_fruit_near ~= nil then
		choices[#choices + 1] = {action = "tree_eat_fruit", text = "tree:eat fruit", args = {tree_fruit_near}}
	end
	if tree_near then
		if hot and day then
			choices[#choices + 1] = {action = "cool_off", text = "tree:cool off", args = {tree_near}}
		end
	end
	if shelter_near then
		if night then
			choices[#choices + 1] = {action = "rest", text = "shelter:rest", args = {shelter_near}}
		end
		if cold and night then
			choices[#choices + 1] = {action = "warm_up", text = "shelter:warm up", args = {shelter_near}}
		end
		if hot and day then
			choices[#choices + 1] = {action = "cool_off", text = "shelter:cooloff", args = {shelter_near}}
		end
		choices[#choices + 1] = {action = "shelter_scrap", text = "shelter:scrap", args = {shelter_near}}
	end
	if fire_near then
		if night then
			choices[#choices + 1] = {action = "rest", text = "fire:rest", args = {fire_near}}
		end
		if cold and night then
			choices[#choices + 1] = {action = "warm_up", text = "fire:warm up", args = {fire_near}}
		end
		choices[#choices + 1] = {action = "fire_scrap", text = "fire:scrap", args = {fire_near}}
	end

	choices[#choices + 1] = {action = "pop_dialog", text = "back"}

	self:push_dialog("interact with", choices)
end
function Player.WorldSys:push_make_dialogue()
	self:push_dialog("what to make..", {
		{action = "fire_make", text = "W1R1 fire"},
		{action = "shelter_make", text = "W8R8 shelter"},
		{action = "pop_dialog", text = "back"},
	})
end
function Player.WorldSys:push_root_action_dialogue()
	self:push_dialog("what to do?", {
		{text = "interact", action = "push_interact_dialogue"},
		{text = "make", action = "push_make_dialogue"},
		{text = "nothing", action = "pop_dialog"},
	})
end

-- progression
function Player.WorldSys:find_in_range(tags)
	local _, entity = self._entity:find_tagged(self.sys_name)
	local x, y = entity.x or 0, entity.y or 0

	local range_x = x - 1
	local range_y = y - 1
	local range_width = 10
	local range_height = 10

	local entity_near = self._entity:find_in(range_x, range_y, range_width, range_height, tags)
	return entity_near
end
function Player.WorldSys:is_in_shade()
	if self:is_nighttime_at_turn(self.state.turn_id) then
		return self:find_in_range({"fire"}) ~= nil
	else
		return self:find_in_range({"tree"}) ~= nil
	end
end
function Player.WorldSys:is_in_shelter()
	return self:is_in_shade() or self:find_in_range({"shelter"}) ~= nil
end
function Player.WorldSys:is_nighttime_at_turn(turn_id)
	return ((turn_id % self.state.turns_per_day)
			> (self.state.turns_per_day * (1 - self.state.fraction_of_day_is_nighttime)))
end
function Player.WorldSys:is_evening_at_turn(turn_id)
	return ((turn_id % self.state.turns_per_day)
			> (self.state.turns_per_day * 0.66 * (1 - self.state.fraction_of_day_is_nighttime)))
end
function Player.WorldSys:get_day_count_at_turn(turn_id)
	return math.floor(turn_id / self.state.turns_per_day) + 1
end
function Player.WorldSys:check_day_night_cycle()
	local should_be_nighttime = self:is_nighttime_at_turn(self.state.turn_id)
	if should_be_nighttime ~= self.state.is_nighttime then
		for entity_id, entity in ipairs(self._entity:get_all_raw()) do
			if entity.destroyed ~= true then
				if entity.night_image_name ~= nil then
					assert(entity.day_image_name ~= nil)

					if should_be_nighttime then
						self._image:entity_set(entity_id, entity.night_image_name, entity)
					else
						self._image:entity_set(entity_id, entity.day_image_name, entity)
					end
				end

				if entity.night_color ~= nil then
					assert(entity.day_color ~= nil)

					if should_be_nighttime then
						self._image:entity_set_color(entity_id, Engine.Core.Shim.unpack(entity.night_color))
					else
						self._image:entity_set_color(entity_id, Engine.Core.Shim.unpack(entity.day_color))
					end
				end
			end
		end

		if should_be_nighttime then
			self._client.clear_color = self.state.night_clear_color
			self:push_dialog("night is here...")
		else
			self._client.clear_color = self.state.day_clear_color
			self:push_dialog("a new day!")
		end

		self.state.is_nighttime = should_be_nighttime
		self.sim:broadcast("on_set_is_nighttime", self.state.is_nighttime)
	end
end
function Player.WorldSys:end_turn(count)
	count = count or 1

	for _ = 1, count do
		self.state.turn_id = self.state.turn_id + 1

		local night = self:is_nighttime_at_turn(self.state.turn_id)
		local evening = self:is_evening_at_turn(self.state.turn_id)
		local day = not night and not evening

		self:offset_hunger(1)

		if self.state.is_raining then
			self:offset_thirst(-1)
		else
			self:offset_thirst(1)
		end

		local rapid_temperature_change = 25
		local fast_temperature_change = 12
		local temperature_change = 7

		local cold = self.state.temperature < level_threshold_mid
		local hot = self.state.temperature > level_threshold_mid
		if cold and day and not self.state.is_raining then
			self:offset_temperature(rapid_temperature_change)
		elseif (hot or (self.state.is_raining)) and night then
			self:offset_temperature(-rapid_temperature_change)
		elseif self:is_in_shade() or (day and self.state.is_raining) then
			if cold then
				self:offset_temperature(temperature_change)
			elseif hot then
				self:offset_temperature(-temperature_change)
			end
		elseif day then
			self:offset_temperature(temperature_change)
		elseif night then
			self:offset_temperature(-fast_temperature_change)
		end

		local damage = 0
		if self.state.hunger >= level_threshold_danger_high then
			damage = damage + 2
		end
		if self.state.thirst >= level_threshold_danger_high then
			damage = damage + 4
		end
		if self.state.temperature >= level_threshold_danger_high then
			damage = damage + 4
		end
		if self.state.temperature <= level_threshold_danger_low then
			damage = damage + 8
		end

		if damage == 0 then
			damage = damage - 1
		end

		self:offset_health(-damage)

		self.sim:broadcast("on_player_end_turn", self.state.turn_id)
	end

	self:check_day_night_cycle()
end

-- events
function Player.WorldSys:on_init()
	Engine.Core.Container.set_defaults(self.state, Player.WorldSys.State.defaults)

	local GameSystems = require("ld50/systems")
	self._data = self.sim:require(GameSystems.Data.WorldSys)
	self._entity = self.sim:require(Engine.Entity.WorldSys)
	self._image = self.sim:require(Engine.Image.WorldSys)
	self._text = self.sim:require(Engine.Text.WorldSys)
	self._template = self.sim:require(Engine.Template.WorldSys)
	self._camera_target = self.sim:require(Engine.CameraTarget.WorldSys)
	self._controller = self.sim:require(Engine.Controller.WorldSys)
	self._client = self.sim:require(Engine.Client.WorldSys)
	self._camera = self.sim:require(Engine.Camera.WorldSys)

	self._entity:tag_bounds_index_add({self.sys_name, "solid"})
	self._template:update("player", {
		tags = {solid = true},
	})

	self._client.clear_color = self.state.day_clear_color

	self.audio = Engine.Client.Wrappers.Audio.new_wav_file{filename = './ld50/data/menu_navigate.wav'}
	self.audio:set_volume{volume = 0.25}
end
function Player.WorldSys:on_start()
	local entity_id = self._entity:find_tagged(self.sys_name)
	assert(entity_id ~= nil)
	self._camera_target:entity_set_default_camera(entity_id)

	self:push_dialog("i must find supplies for a fire\n(W1R1)")
	self:push_dialog("where did i end up...")
end
function Player.WorldSys:on_step()
	if self:step_dialog() == false then
		return
	end

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
	if self._controller:get_pressed(1, "a") then
		self:push_root_action_dialogue()
	end

	local is_turn = false
	for _, entity in ipairs(self._entity:get_all_tagged_array(self.sys_name)) do
		local entity_id = self._entity:find_id(entity)

		if self._entity:find_relative(entity_id, move_x, move_y, {"solid"}) == nil then
			if (move_x ~= 0) or (move_y ~= 0) then
				local base_image_name = "player"
				if move_x > 0 or (move_x == 0 and entity.day_image_name == "player_right") then
					base_image_name = "player_right"
				end
				local night_image_name = base_image_name.."_night"

				local image_name = base_image_name
				if entity.image_name == entity.night_image_name then
					image_name = night_image_name
				end

				self._entity:set(entity_id, {
					x = entity.x + move_x,
					y = entity.y + move_y,
					image_name = image_name,
					day_image_name = base_image_name,
					night_image_name = night_image_name,
				})

				is_turn = true
			end
		end
	end

	if is_turn then
		self:end_turn()
	end

	self.state.health = Math.clamp(
		self.state.health, 0, level_threshold * level_thresholds
	)
	self.state.hunger = Math.clamp(
		self.state.hunger, 0, level_threshold * level_thresholds
	)
	self.state.thirst = Math.clamp(
		self.state.thirst, 0, level_threshold * level_thresholds
	)
	self.state.temperature = Math.clamp(
		self.state.temperature, 0, level_threshold * level_thresholds
	)

	self.state.wood = Math.clamp(self.state.wood, 0, 99)
	self.state.rock = Math.clamp(self.state.rock, 0, 99)
end
function Player.WorldSys:on_draw()
	self:draw_fog()
	self:draw_dialog_ui()
	self:draw_status_ui()
end
function Player.WorldSys:on_set_is_raining(is_raining)
	self.state.is_raining = is_raining
end

Player.GameSys = Engine.Game.Sys.new_metatable(Player.WorldSys.sys_name)
Player.GameSys.WorldSys = Player.WorldSys

return Player
