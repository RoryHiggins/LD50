local Debugging = require("engine/core/debugging")
local Testing = require("engine/core/testing")
local Schema = require("engine/core/schema")
local Container = require("engine/core/container")
local Model = require("engine/core/model")
local Sim = require("engine/engine/sim")
local Client = require("engine/engine/client")
local World = require("engine/engine/world")
local Game = require("engine/engine/game")
local Entity = require("engine/engine/entity")
local Image = require("engine/engine/image")

local debug_checks_enabled = Debugging.debug_checks_enabled
local expensive_debug_checks_enabled = Debugging.expensive_debug_checks_enabled

local math_min = math.min
local math_floor = math.floor

local Animation = {}

Animation.FontType = Model.Enum("ascii")

Animation.Animation = {}
Animation.Animation.Schema = Schema.Object{
	frames = Schema.NonEmptyArray(Schema.LabelString),
}

Animation.Entity = {}
Animation.Entity.Schema = Schema.AllOf(Entity.Entity.Schema, Schema.PartialObject{
	anim_name = Schema.Optional(Schema.LabelString),
	anim_speed = Schema.Optional(Schema.NonNegativeNumber),
	anim_pos = Schema.Optional(Schema.NonNegativeNumber),
	anim_looping = Schema.Optional(Schema.Boolean),
})

Animation.WorldSys = World.Sys.new_metatable("animation")
Animation.WorldSys.tag = "animation"
Animation.WorldSys.State = {}
Animation.WorldSys.State.Schema = Schema.Object{
	animations = Schema.Mapping(Schema.LabelString, Animation.Animation.Schema),
}
Animation.WorldSys.State.defaults = {
	animations = {},
}
Animation.WorldSys.Schema = Schema.AllOf(World.Sys.Schema, Schema.PartialObject{
	state = Animation.WorldSys.State.Schema,
	_client_world = Client.WorldSys.Schema,
	_entity_world = Entity.WorldSys.Schema,
	_image_world = Image.WorldSys.Schema,
	_entity_reindex_required = Schema.Boolean,
})
function Animation.WorldSys:index(anim_name, animation)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Animation.WorldSys.Schema(self))
			assert(Animation.Animation.Schema(animation))
		end
		assert(Schema.LabelString(anim_name))
	end

	animation = animation or self:find(anim_name)

	self.state.animations[anim_name] = animation

	self._entity_reindex_required = true
end
function Animation.WorldSys:index_all()
	if expensive_debug_checks_enabled then
		assert(Animation.WorldSys.Schema(self))
	end

	Container.set_defaults(self.state, Animation.WorldSys.State.defaults)

	for anim_name, animation in pairs(self.state.animations) do
		self:index(anim_name, animation)
	end

	self._entity_reindex_required = true
end
function Animation.WorldSys:set(anim_name, animation)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Animation.WorldSys.Schema(self))
			assert(Animation.Animation.Schema(animation))
		end
		assert(Schema.LabelString(anim_name))
	end

	self:index(anim_name, animation)
end
local anim_frame_image_name_template = "anim_%s_%s"
function Animation.WorldSys:set_frames(anim_name, frame_bounds_array, filename, file_type, grid_width, grid_height)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Animation.WorldSys.Schema(self))
			assert(Schema.NonEmptyArray(Schema.BoundedArray(Schema.Integer, 2, 4))(frame_bounds_array))
		end
		assert(Schema.LabelString(anim_name))
		assert(Schema.String(filename))
		assert(Schema.Optional(Image.FileType.Schema)(file_type))
		assert(Schema.Optional(Schema.NonNegativeInteger)(grid_width))
		assert(Schema.Optional(Schema.NonNegativeInteger)(grid_height))
	end

	local frames = {}
	for i, frame_bounds in ipairs(frame_bounds_array) do
		local frame_image_name = string.format(anim_frame_image_name_template, anim_name, tostring(i))
		self._image_world:set(frame_image_name, {
			filename = filename,
			file_type = file_type,
			u = frame_bounds[1],
			v = frame_bounds[2],
			width = frame_bounds[3] or grid_width,
			height = frame_bounds[4] or grid_height or grid_width,
		})
		frames[i] = frame_image_name
	end

	self:set(anim_name, {frames = frames})
end
function Animation.WorldSys:set_strip(anim_name, u, v, width, height, filename, file_type, grid_width, grid_height)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Animation.WorldSys.Schema(self))
		end
		assert(Schema.LabelString(anim_name))
		assert(Schema.NonNegativeInteger(u))
		assert(Schema.NonNegativeInteger(v))
		assert(Schema.NonNegativeInteger(width))
		assert(Schema.NonNegativeInteger(height))
		assert(Schema.String(filename))
		assert(Schema.Optional(Image.FileType.Schema)(file_type))
		assert(Schema.NonNegativeInteger(grid_width))
		assert(Schema.Optional(Schema.NonNegativeInteger)(grid_height))
	end

	grid_height = grid_height or grid_width

	local frame_bounds = {}
	local cols = math.floor(width / grid_width)
	local rows = math.floor(height / grid_height)
	for row = 0, rows - 1 do
		for col = 0, cols - 1 do
			frame_bounds[#frame_bounds + 1] = {u + (col * grid_width), v + (row * grid_height)}
		end
	end

	self:set_frames(anim_name, frame_bounds, filename, file_type, grid_width, grid_height)
end
function Animation.WorldSys:find(anim_name)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Animation.WorldSys.Schema(self))
		end
		assert(Schema.LabelString(anim_name))
	end

	return self.state.animations[anim_name]
end
function Animation.WorldSys:entity_index(entity_id, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Animation.WorldSys.Schema(self))
			assert(Schema.Optional(Animation.Entity.Schema)(entity))
		end
		assert(Schema.PositiveInteger(entity_id))
	end

	entity = entity or self._entity_world:find(entity_id)
	if entity.tags == nil or entity.tags[self.tag] ~= true then
		return
	end

	local anim_name = entity.anim_name
	if anim_name == nil then
		return
	end

	local animation = self:find(anim_name)
	local frames = animation.frames
	if debug_checks_enabled then
		assert(animation ~= nil)
		assert(#animation.frames > 0)
	end

	local anim_pos = (entity.anim_pos or 0)

	local anim_looping = entity.anim_looping or true
	entity.anim_looping = anim_looping
	if anim_looping then
		anim_pos = anim_pos % #frames
	else
		anim_pos = math_min(anim_pos, #frames - 1)
	end

	entity.anim_pos = anim_pos

	local frame_id = math_floor(anim_pos) + 1
	self._image_world:entity_set(entity_id, frames[frame_id], entity)
end
function Animation.WorldSys:entity_index_all()
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Animation.WorldSys.Schema(self))
		end
	end

	for entity_id, entity in ipairs(self._entity_world:get_all_raw()) do
		if entity.destroyed ~= true and entity.tags ~= nil and entity.tags[self.tag] == true then
			self:entity_index(entity_id, entity)
		end
	end

	self._entity_reindex_required = false
end
local anim_tag = Animation.WorldSys.tag
function Animation.WorldSys:entity_set(entity_id, anim_name, anim_speed, anim_looping, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Animation.WorldSys.Schema(self))
			assert(Schema.Optional(Animation.Entity.Schema)(entity))
		end
		assert(Schema.PositiveInteger(entity_id))
		assert(Schema.Optional(Schema.LabelString)(anim_name))
		assert(Schema.Optional(Schema.Boolean)(anim_looping))
		assert(Schema.Optional(Schema.NonNegativeNumber)(anim_speed))
		assert(anim_name == nil or self:find(anim_name) ~= nil)

		assert(self.sim.status == Sim.Status.started)
	end

	entity = entity or self._entity_world:find(entity_id)

	entity.anim_name = anim_name
	if anim_name == nil then
		if debug_checks_enabled then
			assert(anim_speed == nil or anim_speed == 0)
		end

		entity.anim_pos = nil
		entity.anim_speed = nil
		entity.anim_looping = nil

		if entity.tags ~= nil and entity.tags[anim_tag] == true then
			self._entity_world:untag(entity_id, {anim_tag}, entity)
		end
	else
		entity.anim_pos = 0
		entity.anim_speed = anim_speed or 1
		entity.anim_looping = anim_looping or true

		if entity.tags == nil or entity.tags[anim_tag] ~= true then
			self._entity_world:tag(entity_id, {anim_tag}, entity)
		end
	end
end
function Animation.WorldSys:entity_unset(entity_id, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Animation.WorldSys.Schema(self))
			assert(Schema.Optional(Animation.Entity.Schema)(entity))
		end
		assert(Schema.PositiveInteger(entity_id))

		assert(self.sim.status == Sim.Status.started)
	end

	entity = entity or self._entity_world:find(entity_id)
	self:entity_set(entity_id, nil, nil, nil, entity)
end
function Animation.WorldSys:entity_get_stopped(entity_id, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Animation.WorldSys.Schema(self))
			assert(Schema.Optional(Animation.Entity.Schema)(entity))
		end
		assert(Schema.PositiveInteger(entity_id))

		assert(self.sim.status == Sim.Status.started)
	end

	entity = entity or self._entity_world:find(entity_id)
	local anim_name = entity.anim_name
	if anim_name == nil then
		return true
	end

	if entity.anim_speed == 0 then
		return true
	end

	return (
		entity.anim_looping ~= false
		and entity.anim_pos ~= nil
		and entity.anim_pos == #(self:find(anim_name).frames) - 1)
end
function Animation.WorldSys:entity_stop(entity_id, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Animation.WorldSys.Schema(self))
			assert(Schema.Optional(Animation.Entity.Schema)(entity))
		end
		assert(Schema.PositiveInteger(entity_id))

		assert(self.sim.status == Sim.Status.started)
	end

	entity = entity or self._entity_world:find(entity_id)
	local anim_name = entity.anim_name
	if anim_name ~= nil then
		entity.anim_speed = 0
	end
end
function Animation.WorldSys:on_init()
	Container.set_defaults(self.state, Animation.WorldSys.State.defaults)

	self._client_world = self.sim:require(Client.WorldSys)
	self._entity_world = self.sim:require(Entity.WorldSys)
	self._image_world = self.sim:require(Image.WorldSys)

	self._entity_reindex_required = true

	self:index_all()

	if expensive_debug_checks_enabled then
		assert(Animation.WorldSys.Schema(self))
	end
end
function Animation.WorldSys:on_step()
	for _, entity in ipairs(self._entity_world:get_all_tagged_raw(self.tag)) do
		local anim_name = entity.anim_name
		if anim_name ~= nil then
			local animation = self:find(anim_name)
			local frames = animation.frames
			local anim_speed = entity.anim_speed or 1
			local anim_pos = ((entity.anim_pos or 0) + anim_speed)

			local anim_looping = entity.anim_looping or true
			entity.anim_looping = anim_looping
			if anim_looping then
				anim_pos = anim_pos % #frames
			else
				anim_pos = math_min(anim_pos, #frames - 1)
			end

			entity.anim_pos = anim_pos

			local entity_id = self._entity_world:find_id(entity)
			local frame_id = math_floor(anim_pos) + 1
			self._image_world:entity_set(entity_id, frames[frame_id], entity)
		end
	end
end
function Animation.WorldSys:on_entity_index(entity_id, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Animation.WorldSys.Schema(self))
			assert(Animation.Entity.Schema(entity))
		end
		assert(Schema.PositiveInteger(entity_id))
	end

	self:entity_index(entity_id, entity)
end

Animation.GameSys = Game.Sys.new_metatable("animation")
Animation.GameSys.WorldSys = Animation.WorldSys

Animation.tests = Testing.add_suite("engine.animation", {
	run_game = function()
		local game = Game.Game.new({client = {visible = false}})
		-- local game = Game.Game.new()  -- to test with a non-headless client

		local world_game = game:require(World.GameSys)
		game:require(Image.GameSys)
		world_game:require_world_sys(Animation.WorldSys)

		local filename = "./examples/engine_test/data/sprites.png"
		game:start()

		local world = world_game.world
		local entity_world = world:get(Entity.WorldSys)
		local anim_world = world:get(Animation.WorldSys)

		local anim_frames = {
			{80, 40},
			{88, 40},
			{96, 40},
			{104, 40},
		}
		local anim_name = "lava"
		anim_world:set_frames(anim_name, anim_frames, filename, "png", 8)

		local entity_id = entity_world:add({
			x = 48, y = 48, width = 8, height = 8,
			tags = {[Animation.WorldSys.tag] = true},
			anim_name = anim_name, anim_speed = 0.2
		})
		assert(anim_world:entity_get_stopped(entity_id) == false)
		anim_world:entity_stop(entity_id)
		assert(anim_world:entity_get_stopped(entity_id) == true)

		anim_world:entity_unset(entity_id)
		assert(anim_world:entity_get_stopped(entity_id) == true)

		anim_world:entity_set(entity_id, anim_name, 0.2, true)
		assert(anim_world:entity_get_stopped(entity_id) == false)

		for _ = 1, 12 do
			game:step()
		end

		game:stop()
		game:finalize()
	end,
})

return Animation
