local Engine = require("engine/engine")

local Math = Engine.Core.Math

local level_threshold = 100
local level_thresholds = 4

local Player = {}
Player.WorldSys = Engine.World.Sys.new_metatable("player")
Player.WorldSys.State = {}
Player.WorldSys.State.defaults = {
	-- actual state
	health = 350,
	hunger = 300,
	thirst = 200,
	temperature = 200,

	wood = 0,
	rock = 0,

	dialog_stack = {},
	dialog_choice = 1,

	is_nighttime = false,
	turn_id = 0,

	-- configuration
	day_clear_color = {57, 120, 168, 255},
	night_clear_color = {57, 49, 75, 255},
	turns_per_day = 140,
	fraction_of_day_is_nighttime = 0.4,
	prompt_toggle_steps = 30,
}
-- rendering
function Player.WorldSys:draw_dialog_ui()
	if #self.state.dialog_stack == 0 then
		local x, y = self._client:camera_get_pos("default")
		local z = -100
		local width, height = self._client:camera_get_size("default")

		local text = "z=action"
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
			text = " "..text
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
function Player.WorldSys:draw_blackout()
	if self.state.is_nighttime then
		local _, entity = self._entity:find_tagged(self.sys_name)
		local player_x, player_y = entity.x or 0, entity.y or 0

		local start_x, start_y = self._client:camera_get_pos("default")
		local width, height = self._client:camera_get_size("default")
		start_x, start_y = math.floor(start_x / 8) * 8, math.floor(start_y / 8) * 8

		local light_tags = {"light"}
		local dim_dist = 8
		local sight = 40
		local dim_sight = 20
		local blackout_half_image = "blackout_half_"..(1 + (math.floor(self.sim.step_id / 10) % 2))
		local image_world = self._image
		for x = start_x, start_x + width, 8 do
			for y = start_y, start_y + height, 8 do
				if Math.distance(x, y, player_x, player_y) >= sight then
					image_world:draw("blackout_full", x, y, 8, 8, 255,255,255,255, -99)
				elseif self._entity:find_in(x + 4 - dim_dist, y + 4 - dim_dist, dim_dist * 2, dim_dist * 2, light_tags) == nil
				and Math.distance(x, y, player_x, player_y) >= dim_sight then
						image_world:draw(blackout_half_image, x, y, 8, 8, 255,255,255,255, -99)
					-- end
				-- else
				-- 	image_world:draw("blackout_full", x, y, 8, 8, 255,255,255,255, -99)
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
		if self.state.health <= level_threshold then
			vertex_array:add_rect(
				status_x, status_y, status_x + 8, status_y + 8, 255,0,0,255, ui_z - 1)
		end
		if self.state.temperature <= level_threshold
		or self.state.temperature >= (level_thresholds - 1) * level_threshold then
			vertex_array:add_rect(
				status_x + 8, status_y, status_x + 16, status_y + 8, 255,0,0,255, ui_z - 1)
		end
		if self.state.thirst <= level_threshold then
			vertex_array:add_rect(
				status_x + 16, status_y, status_x + 24, status_y + 8, 255,0,0,255, ui_z - 1)
		end
		if self.state.hunger <= level_threshold then
			vertex_array:add_rect(
				status_x + 24, status_y, status_x + 32, status_y + 8, 255,0,0,255, ui_z - 1)
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
function Player.WorldSys:perform_dialog_action(action)
	assert(type(action) == "string")

	if self[action] then
		self[action](self)
	else
		Engine.Core.Logging.warning("Unknown action=%s", action)
		self:pop_dialog()
	end
end
function Player.WorldSys:noop_dialog()
end
function Player.WorldSys:step_dialog()
	if #self.state.dialog_stack == 0 then
		return true
	end

	local dialog = self.state.dialog_stack[#self.state.dialog_stack]
	assert(dialog.text ~= nil)
	assert(dialog.choices ~= nil)

	if self._controller:get_pressed(1, "up") then
		self.state.dialog_choice = self.state.dialog_choice - 1
		if self.state.dialog_choice < 1 then
			self.state.dialog_choice = #dialog.choices
		end
	end
	if self._controller:get_pressed(1, "down") then
		self.state.dialog_choice = self.state.dialog_choice + 1
		if self.state.dialog_choice > #dialog.choices then
			self.state.dialog_choice = 1
		end
	end
	self.state.dialog_choice = Math.clamp(self.state.dialog_choice, 1, #dialog.choices)

	local pop_dialog = false
	if self._controller:get_pressed(1, "a") then
		if #dialog.choices == 0 then
			pop_dialog = true
		else
			self:perform_dialog_action(dialog.choices[self.state.dialog_choice].action)
		end
	end

	if self._controller:get_pressed(1, "b") then
		pop_dialog = true
	end

	if pop_dialog == true then
		self:pop_dialog()
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
	return math.floor(attribute_value / level_threshold)
end
function Player.WorldSys:clamped_attr_add(attribute_value, amount)
	return Math.clamp(
		attribute_value + amount, 0, level_threshold * level_thresholds - 1
	)
end
function Player.WorldSys:add_hunger(amount)
	local hunger_level = self.state.hunger * level_threshold

	local hunger_after = self:clamped_attr_add(self.state.hunger, amount)
	local hunger_level_after = hunger_after * level_threshold
	local hunger_warning_fraction = 0.75
	if (hunger_level / level_thresholds) < hunger_warning_fraction then
		if (hunger_level_after / level_thresholds) >= hunger_warning_fraction then
			self:push_dialog("maybe those trees will have fruit")
			self:push_dialog("starving...")
		end
	end
	self.state.hunger = hunger_after
end
function Player.WorldSys:add_thirst(amount)
	local thirst_level = self.state.thirst * level_threshold

	local thirst_after = self:clamped_attr_add(self.state.thirst, amount)
	local thirst_level_after = thirst_after * level_threshold
	local thirst_warning_fraction = 0.75
	if (thirst_level / level_thresholds) < thirst_warning_fraction then
		if (thirst_level_after / level_thresholds) >= thirst_warning_fraction then
			self:push_dialog("must be fresh water somewhere")
			self:push_dialog("so thirsty")
		end
	end
	self.state.thirst = thirst_after
end
function Player.WorldSys:add_health(amount)
	local health_level = self.state.health * level_threshold

	local health_after = self:clamped_attr_add(self.state.health, amount)
	local health_level_after = health_after * level_threshold
	local health_warning_fraction = 0.25
	if (health_level / level_thresholds) > health_warning_fraction then
		if (health_level_after / level_thresholds) <= health_warning_fraction then
			self:push_dialog("i cant much more take this")
		end
	end

	if health_after == 0 then
		self:clear_dialog()
		self:push_dialog("so this is the end", {
			{action = "restart", text = "try again"},
			{action = "quit", text = "quit"},
		})
	end

	self.state.health = health_after
end
function Player.WorldSys:add_temperature(amount)
	local health_level = self.state.health * level_threshold

	local health_after = self:clamped_attr_add(self.state.health, amount)
	local health_level_after = health_after * level_threshold
	local hot_warning_fraction = 0.75
	if (health_level / level_thresholds) < hot_warning_fraction then
		if (health_level_after / level_thresholds) >= hot_warning_fraction then
			self:push_dialog("maybe i can find some shelter from the sun")
			self:push_dialog("too hot")
		end
	end
	local cold_warning_fraction = 0.25
	if (health_level / level_thresholds) > cold_warning_fraction then
		if (health_level_after / level_thresholds) <= cold_warning_fraction then
			self:push_dialog("maybe i can make a fire")
			self:push_dialog("its freezing")
		end
	end

	if health_after == 0 then
		self:clear_dialog()
		self:push_dialog("so this is the end", {
			{action = "restart", text = "try again"},
			{action = "quit", text = "quit"},
		})
	end

	self.state.health = health_after
end

-- action menus
function Player.WorldSys:push_interact_dialogue()
	local _, entity = self._entity:find_tagged(self.sys_name)
	local x, y = entity.x or 0, entity.y or 0

	local range_x = x - 1
	local range_y = y - 1
	local range_width = 10
	local range_height = 10

	local choices = {}
	if self._entity:find_in(range_x, range_y, range_width, range_height, {"fresh_water"}) ~= nil then
		choices[#choices + 1] = {action = "drink_water", text = "drink(puddle)"}
	end
	if self._entity:find_in(range_x, range_y, range_width, range_height, {"fruit"}) ~= nil then
		choices[#choices + 1] = {action = "eat_fruit", text = "eat(fruit)"}
	end
	if self._entity:find_in(range_x, range_y, range_width, range_height, {"wood"}) ~= nil then
		choices[#choices + 1] = {action = "take_wood", text = "take(wood)"}
	end
	if self._entity:find_in(range_x, range_y, range_width, range_height, {"rock"}) ~= nil then
		choices[#choices + 1] = {action = "take_rock", text = "take(rock)"}
	end
	if self._entity:find_in(range_x, range_y, range_width, range_height, {"shelter"}) ~= nil
	and not self.state.is_nighttime then
		choices[#choices + 1] = {action = "take_shelter", text = "use(shelter)"}
	end
	if self._entity:find_in(range_x, range_y, range_width, range_height, {"shelter"}) ~= nil
	and self.state.is_nighttime then
		choices[#choices + 1] = {action = "rest_shelter", text = "rest(shelter)"}
	end
	if self._entity:find_in(range_x, range_y, range_width, range_height, {"fire"}) ~= nil
	and self.state.is_nighttime then
		choices[#choices + 1] = {action = "rest_fire", text = "rest(fire)"}
	end
	if self._entity:find_in(range_x, range_y, range_width, range_height, {"shelter"}) ~= nil then
		choices[#choices + 1] = {action = "scrap_shelter", text = "scrap(shelter)"}
	end

	choices[#choices + 1] = {action = "pop_dialog", text = "back"}

	self:push_dialog("interact with", choices)
end
function Player.WorldSys:push_make_dialogue()
	self:push_dialog("what to make..", {
		{action = "make_fire", text = "W1R1 fire"},
		{action = "make_shelter", text = "W8R1 shelter"},
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

function Player.WorldSys:is_nighttime_at_turn(turn_id)
	return ((turn_id % self.state.turns_per_day)
			> (self.state.turns_per_day * (1 - self.state.fraction_of_day_is_nighttime)))
end
function Player.WorldSys:is_evening_at_turn(turn_id)
	return ((turn_id % self.state.turns_per_day)
			> (self.state.turns_per_day * 0.75 * (1 - self.state.fraction_of_day_is_nighttime)))
end
function Player.WorldSys:get_day_count_at_turn(turn_id)
	return math.floor(turn_id / self.state.turns_per_day) + 1
end
function Player.WorldSys:enforce_day_night_cycle()
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
	end
end
function Player.WorldSys:end_turn()
	self.state.turn_id = self.state.turn_id + 1

	self:enforce_day_night_cycle()
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
end
function Player.WorldSys:on_start()
	local entity_id = self._entity:find_tagged(self.sys_name)
	assert(entity_id ~= nil)
	self._camera_target:entity_set_default_camera(entity_id)

	self:push_dialog("i must find supplies for a fire\n(W1R1)")
	self:push_dialog("where did I end up...")
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
			self._entity:set_pos(
				entity_id,
				entity.x + move_x,
				entity.y + move_y
			)
			is_turn = is_turn or (move_x ~= 0) or (move_y ~= 0)
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
end
function Player.WorldSys:on_draw()
	self:draw_blackout()
	self:draw_dialog_ui()
	self:draw_status_ui()
end

Player.GameSys = Engine.Game.Sys.new_metatable(Player.WorldSys.sys_name)
Player.GameSys.WorldSys = Player.WorldSys

return Player
