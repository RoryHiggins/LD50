local Shim = require("engine/core/shim")
local Math = require("engine/core/math")
local Debugging = require("engine/core/debugging")
local Logging = require("engine/core/logging")
local Testing = require("engine/core/testing")
local Schema = require("engine/core/Schema")
local Container = require("engine/core/container")
local Sim = require("engine/engine/sim")
local Client = require("engine/engine/client")
local World = require("engine/engine/world")

local debug_checks_enabled = Debugging.debug_checks_enabled
local expensive_debug_checks_enabled = Debugging.expensive_debug_checks_enabled

local Entity = {}

Entity.Entity = {}
Entity.Entity.max_tag_id = Client.Wrappers.EntityIndex.get_max_tag_id() - 1  -- -1 for lua array indexing
Entity.Entity.DestroyedSchema = Schema.Object{
	destroyed = Schema.Const(true),
}
Entity.Entity.ExistsSchema = Schema.AllOf(Schema.SerializableObject, Schema.PartialObject{
	destroyed = Schema.Nil,
	name = Schema.Optional(Schema.LabelString),
	tags = Schema.Optional(Schema.Mapping(Schema.LabelString, Schema.Const(true))),
	x = Schema.Optional(Schema.Integer),
	y = Schema.Optional(Schema.Integer),
	width = Schema.Optional(Schema.NonNegativeInteger),
	height = Schema.Optional(Schema.NonNegativeInteger),
	sprite_name = Schema.Optional(Schema.LabelString),
	z = Schema.Optional(Schema.Number),
	r = Schema.Optional(Schema.BoundedInteger(0, 255)),
	g = Schema.Optional(Schema.BoundedInteger(0, 255)),
	b = Schema.Optional(Schema.BoundedInteger(0, 255)),
	a = Schema.Optional(Schema.BoundedInteger(0, 255)),
	translate_x = Schema.Optional(Schema.Number),
	translate_y = Schema.Optional(Schema.Number),
	scale_x = Schema.Optional(Schema.BoundedNumber(Math.epsilon, Math.integer_max)),
	scale_y = Schema.Optional(Schema.BoundedNumber(Math.epsilon, Math.integer_max)),
})
Entity.Entity.Schema = Schema.OneOf(Entity.Entity.DestroyedSchema, Entity.Entity.ExistsSchema)
-- Entity.Entity.Schema = Entity.Entity.ExistsSchema
Entity.Entity.defaults = {}

Entity.WorldSys = World.Sys.new_metatable("entity")
Entity.WorldSys.State = {}
Entity.WorldSys.State.Schema = Schema.Object{
	entities = Schema.Array(Entity.Entity.Schema),
}
Entity.WorldSys.State.defaults = {
	entities = {},
}
Entity.WorldSys.Schema = Schema.AllOf(World.Sys.Schema, Schema.PartialObject{
	state = Entity.WorldSys.State.Schema,
	_tag_id_to_tag = Schema.BoundedArray(Schema.LabelString, 0, Entity.Entity.max_tag_id),
	_tag_to_tag_id = Schema.Mapping(Schema.LabelString, Schema.BoundedInteger(0, Entity.Entity.max_tag_id)),

	_entity_index = Client.Wrappers.Schema("EntityIndex"),
	_entity_ids_free = Schema.Mapping(Schema.PositiveInteger, Schema.Const(true)),
	_entity_to_entity_id = Schema.Mapping(Entity.Entity.Schema, Schema.PositiveInteger),
	_tag_to_entities = Schema.Mapping(Schema.LabelString, Schema.Array(Entity.Entity.Schema)),
	-- for efficient cleanup of _tag_to_entities
	_entity_id_to_tag_indices = Schema.Mapping(
		Schema.PositiveInteger, Schema.Mapping(Schema.LabelString, Schema.PositiveInteger)),
	-- _sprite_game = Optional(Sprite.GameSys.Schema),  -- TODO once sprite sys is integrated
})
function Entity.WorldSys:add(entity)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.Optional(Entity.Entity.ExistsSchema)(entity))
		assert(self.sim.status == Sim.Status.started)
	end

	local entities = self.state.entities
	local entity_id = next(self._entity_ids_free) or #entities + 1
	entity = entity or {}

	self:index(entity_id, entity)

	if expensive_debug_checks_enabled then
		assert(Schema.PositiveInteger(entity_id))
		assert(Entity.WorldSys.Schema(self))
	end

	return entity_id, entity
end
function Entity.WorldSys:set(entity_id, state, entity)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
		assert(Entity.Entity.ExistsSchema(state))
		assert(not state.destroyed)
		assert(entity == nil or self.state.entities[entity_id] == entity)
		assert(entity == nil or not entity.destroyed)
		assert(self.sim.status == Sim.Status.started)
	end

	entity = entity or self:find(entity_id)

	Container.update(entity, state)

	self:index(entity_id, entity)

	if expensive_debug_checks_enabled then
		assert(Entity.Entity.Schema(entity))
	end
end
function Entity.WorldSys:destroy(entity_id, entity)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
		assert(entity == nil or self.state.entities[entity_id] == entity)
		assert(self.sim.status == Sim.Status.started)
	end

	entity = entity or self:find(entity_id)

	for key, _ in pairs(entity) do
		entity[key] = nil
	end
	entity.destroyed = true

	self:index(entity_id, entity)

	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Entity.Entity.DestroyedSchema(entity))
	end
end
function Entity.WorldSys:index(entity_id, entity)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
		assert(Schema.Optional(Entity.Entity.Schema)(entity))
		assert(self.sim.status == Sim.Status.started)
	end

	local entities = self.state.entities
	local entity_to_entity_id = self._entity_to_entity_id
	entity = entity or entities[entity_id]
	entities[entity_id] = entity
	entity_to_entity_id[entity] = entity_id
	self._entity_ids_free[entity_id] = entity.destroyed

	local entity_id_to_tag_indices = self._entity_id_to_tag_indices
	local entity_tag_indices = entity_id_to_tag_indices[entity_id] or {}
	entity_id_to_tag_indices[entity_id] = entity_tag_indices

	-- check for any removed tags
	local entity_tags = entity.tags or {}
	local removed_tags = {}
	for tag, _ in pairs(entity_tag_indices) do
		if entity_tags[tag] == nil then
			removed_tags[#removed_tags + 1] = tag
		end
	end
	if #removed_tags > 0 then
		self:untag(entity_id, removed_tags, entity)
	end

	-- check for any added tags, and for bounds indexed tag ids to set
	local tag_to_tag_id = self._tag_to_tag_id
	local bounds_indexed_tag_ids = {}
	local added_tags = {}
	for tag, value in pairs(entity_tags) do
		if value == true then
			local tag_id = tag_to_tag_id[tag]
			if tag_id ~= nil then
				bounds_indexed_tag_ids[#bounds_indexed_tag_ids + 1] = tag_id
			end

			if entity_tag_indices[tag] == nil then
				added_tags[#added_tags + 1] = tag
			end
		end
	end
	if #added_tags > 0 then
		self:tag(entity_id, added_tags, entity)
	end

	local x1, y1 = entity.x or 0, entity.y or 0
	local x2, y2 = x1 + (entity.width or 0), y1 + (entity.height or 0)

	self._entity_index:set(
		entity_id,
		-- bounds
		x1, y1, x2, y2,
		-- sprite - TODO extract UVs once sprite sys hooked up
		0, 0, 0, 0,
		entity.r or 255, entity.g or 255, entity.b or 255, entity.a or 255,
		entity.z or 0,
		-- sprite transform - TODO extract once sprite sys hooked up
		entity.scale_x or 1, 0, 0, 0,
		0, entity.scale_y or 1, 0, 0,
		0, 0, 1, 0,
		entity.translate_x or 0, entity.translate_y or 0, 0, 1,
		-- tags
		Shim.unpack(bounds_indexed_tag_ids)
	)

	if entity.destroyed then
		entity_to_entity_id[entity] = nil
		entity_id_to_tag_indices[entity_id] = nil
	end

	self.sim:broadcast("on_entity_index", entity_id, entity)

	if expensive_debug_checks_enabled then
		assert(Entity.Entity.Schema(entity))
	end
end
function Entity.WorldSys:index_all()
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
	end

	self._entity_index = Client.Wrappers.EntityIndex.new{}
	self._entity_ids_free = {}
	self._entity_to_entity_id = {}
	self._tag_to_entities = {}
	self._entity_id_to_tag_indices = {}

	for entity_id, entity in ipairs(self.state.entities) do
		self:index(entity_id, entity)
	end

	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
	end
end
function Entity.WorldSys:find(entity_id)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
		assert(self.sim.status == Sim.Status.started)
	end

	local entity = self.state.entities[entity_id]
	if entity == nil or entity.destroyed then
		return nil
	end

	return entity
end
function Entity.WorldSys:tag_is_set(entity_id, tags, entity)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
		assert(Schema.Optional(Entity.Entity.Schema)(entity))
		assert(Schema.NonEmptyArray(Schema.LabelString)(tags))
		assert(self.sim.status == Sim.Status.started)
	end

	entity = entity or self:find(entity_id)
	local entity_tags = entity.tags
	if entity_tags == nil then
		return false
	end

	for _, tag in ipairs(tags) do
		if entity_tags[tag] ~= true then
			return false
		end
	end
	return true
end
function Entity.WorldSys:tag(entity_id, tags, entity)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
		assert(Schema.NonEmptyArray(Schema.LabelString)(tags))
		assert(Schema.Optional(Entity.Entity.Schema)(entity))
		assert(self.sim.status == Sim.Status.started)
	end

	entity = entity or self:find(entity_id)
	local entity_tags = entity.tags or {}
	entity.tags = entity_tags

	local entity_id_to_tag_indices = self._entity_id_to_tag_indices
	local entity_tag_indices = entity_id_to_tag_indices[entity_id] or {}
	entity_id_to_tag_indices[entity_id] = entity_tag_indices

	local tag_to_entities = self._tag_to_entities
	local tag_to_tag_id = self._tag_to_tag_id
	local added_bounds_indexed_tag_ids = {}
	local added_tags = {}
	for _, tag in ipairs(tags) do
		if entity_tag_indices[tag] == nil then
			local tag_id = tag_to_tag_id[tag]
			if tag_id ~= nil then
				added_bounds_indexed_tag_ids[#added_bounds_indexed_tag_ids + 1] = tag_id
			end

			local tag_entities = tag_to_entities[tag]
			if tag_entities == nil then
				tag_entities = {}
				tag_to_entities[tag] = tag_entities
			end

			local tag_index = #tag_entities + 1
			tag_entities[tag_index] = entity
			entity_tag_indices[tag] = tag_index
			entity_tags[tag] = true

			added_tags[#added_tags + 1] = tag
		end
	end

	if #added_bounds_indexed_tag_ids > 0 then
		for _, tag_id in ipairs({self._entity_index:get_tags(entity_id)}) do
			added_bounds_indexed_tag_ids[#added_bounds_indexed_tag_ids + 1] = tag_id
		end

		self._entity_index:set_tags(entity_id, Shim.unpack(added_bounds_indexed_tag_ids))
	end

	if #added_tags > 0 then
		self.sim:broadcast("on_entity_tag", entity_id, added_tags, entity)
	end
end
function Entity.WorldSys:untag(entity_id, tags, entity)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
		assert(Schema.NonEmptyArray(Schema.LabelString)(tags))
		assert(Schema.Optional(Entity.Entity.Schema)(entity))
		assert(self.sim.status == Sim.Status.started)
	end

	entity = entity or self:find(entity_id)
	local entity_tags = entity.tags

	local entity_to_entity_id = self._entity_to_entity_id
	local entity_id_to_tag_indices = self._entity_id_to_tag_indices
	local entity_tag_indices = entity_id_to_tag_indices[entity_id]
	if entity_tag_indices == nil then
		return
	end

	entity_id_to_tag_indices[entity_id] = entity_tag_indices

	local tag_to_entities = self._tag_to_entities
	local tag_to_tag_id = self._tag_to_tag_id
	local removed_bounds_indexed_tag_ids = {}
	local removed_tags = {}
	for _, tag in ipairs(tags) do
		local tag_index = entity_tag_indices[tag]
		if tag_index ~= nil then
			local tag_id = tag_to_tag_id[tag]
			if tag_id ~= nil then
				removed_bounds_indexed_tag_ids[tag_id] = true
			end

			local tag_entities = tag_to_entities[tag]

			local swap_tag_index = #tag_entities
			local swap_entity = tag_entities[swap_tag_index]
			local swap_entity_id = entity_to_entity_id[swap_entity]
			local swap_entity_tag_indices = entity_id_to_tag_indices[swap_entity_id]

			if entity_tags ~= nil then
				entity_tags[tag] = nil
			end

			swap_entity_tag_indices[tag] = tag_index
			tag_entities[tag_index] = swap_entity
			tag_entities[swap_tag_index] = nil
			entity_tag_indices[tag] = nil

			removed_tags[#removed_tags + 1] = tag
		end
	end

	if #removed_bounds_indexed_tag_ids > 0 then
		local bounds_indexed_tag_ids = {}
		for _, tag_id in ipairs({self._entity_index:get_tags(entity_id)}) do
			if removed_bounds_indexed_tag_ids[tag_id] ~= true then
				bounds_indexed_tag_ids[#bounds_indexed_tag_ids + 1] = tag_id
			end
		end

		self._entity_index:set_tags(entity_id, Shim.unpack(bounds_indexed_tag_ids))
	end

	if #removed_tags > 0 then
		self.sim:broadcast("on_entity_untag", entity_id, removed_tags, entity)
	end
end
function Entity.WorldSys:tag_get_all(tag)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.LabelString(tag))
		assert(self.sim.status == Sim.Status.started)
	end

	local tag_to_entities = self._tag_to_entities
	local tag_entities = tag_to_entities[tag] or {}
	tag_to_entities[tag] = tag_entities

	return tag_entities
end
function Entity.WorldSys:bounds_get(entity_id, entity)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
		assert(Schema.Optional(Entity.Entity.Schema)(entity))
		assert(self.sim.status == Sim.Status.started)
	end

	entity = entity or self:find(entity_id)

	return entity.x or 0, entity.y or 0, entity.width or 0, entity.height or 0
end
function Entity.WorldSys:bounds_set(entity_id, x, y, width, height, entity)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
		assert(Schema.Integer(x))
		assert(Schema.Integer(y))
		assert(Schema.Integer(width))
		assert(Schema.Integer(height))
		assert(Schema.Optional(Entity.Entity.Schema)(entity))
		assert(self.sim.status == Sim.Status.started)
	end

	entity = entity or self:find(entity_id)

	entity.x = x
	entity.y = y
	entity.width = width
	entity.height = height

	local x1, y1 = x, y
	local x2, y2 = x1 + (width), y1 + (height)

	self._entity_index:set_bounds(entity_id, x1, y1, x2, y2)
end
function Entity.WorldSys:bounds_index_tag(tag, allow_failure)
	if debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.LabelString(tag))
		assert(Schema.Optional(Schema.Boolean)(allow_failure))
		assert(self.sim.status == Sim.Status.new)
	end

	local tag_to_tag_id = self._tag_to_tag_id
	local tag_id_to_tag = self._tag_id_to_tag
	local tag_id = tag_to_tag_id[tag]
	if tag_id ~= nil then
		return tag_id
	end

	tag_id = #tag_id_to_tag + 1
	if tag_id > Entity.Entity.max_tag_id then
		local msg = "Max tag ids reached, cannot add "..tag
		if allow_failure == true then
			Logging.warning(msg)
			return
		else
			error(msg)
		end
	end

	tag_id_to_tag[tag_id] = tag
	tag_to_tag_id[tag] = tag_id

	return tag_id
end
function Entity.WorldSys:get_all_raw()
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(self.sim.status == Sim.Status.started)
	end

	return self.state.entities
end
function Entity.WorldSys:get_max_id()
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(self.sim.status == Sim.Status.started)
	end

	return #self.state.entities
end
function Entity.WorldSys:on_init()
	Container.set_defaults(self.state, Entity.WorldSys.State.defaults)

	self._tag_id_to_tag = {}
	self._tag_to_tag_id = {}

	self._entity_index = Client.Wrappers.EntityIndex.new{}
	self._entity_ids_free = {}
	self._entity_to_entity_id = {}
	self._tag_to_entities = {}
	self._entity_id_to_tag_indices = {}

	if debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
	end
end
function Entity.WorldSys:on_start()
	self:index_all()
end

Entity.tests = Testing.add_suite("engine.entity", {
	add = function()
		local world = World.World.new()
		local entity_world = world:require(Entity.WorldSys)
		world:start()

		local state = {x = 0, y = 0, width = 8, height = 8, tags = {hello = true}, blah = {1, 2, 3}}
		local entity_id = entity_world:add(state)
		Container.assert_equal(state, entity_world:find(entity_id))
	end,
	set = function()
		local world = World.World.new()
		local entity_world = world:require(Entity.WorldSys)
		world:start()

		Testing.assert_fails(function()
			entity_world:set(0, {})
		end)

		local entity_id = entity_world:add{}
		Container.assert_equal({}, entity_world:find(entity_id))

		local state = {x = 0, y = 0, width = 8, height = 8, tags = {hello = true}, blah = {1, 2, 3}}
		entity_world:set(entity_id, state)
		Container.assert_equal(state, entity_world:find(entity_id))
	end,
	destroy = function()
		local world = World.World.new()
		local entity_world = world:require(Entity.WorldSys)
		world:start()

		local entity_id = entity_world:add{}
		Container.assert_equal({}, entity_world:find(entity_id))

		entity_world:destroy(entity_id)
		Container.assert_equal(nil, entity_world:find(entity_id))

		Testing.assert_fails(function()
			entity_world:destroy(entity_id)
		end)
	end,
	index_tags = function()
		local TestSys = World.Sys.new_metatable("test")
		local entity_index_patch = Testing.CallWatcher.patch(TestSys, "on_entity_index")
		local entity_tag_patch = Testing.CallWatcher.patch(TestSys, "on_entity_tag")
		local entity_untag_patch = Testing.CallWatcher.patch(TestSys, "on_entity_untag")

		local world = World.World.new()
		local entity_world = world:require(Entity.WorldSys)
		local test_world = world:require(TestSys)

		local tag = "hello"
		entity_world:bounds_index_tag(tag)

		world:start()

		local entity_id = entity_world:add{}
		local entity = entity_world:find(entity_id)
		entity_index_patch:assert_called_once()
		entity_tag_patch:assert_not_called()
		entity_untag_patch:assert_not_called()
		entity_index_patch:reset()

		entity_world:index(entity_id)
		entity_index_patch:assert_called_once()
		entity_tag_patch:assert_not_called()
		entity_untag_patch:assert_not_called()
		entity_index_patch:reset()
		Container.assert_equal({entity_world._entity_index:get_tags(entity_id)}, {})

		entity.tags = {[tag] = true}
		entity_world:index(entity_id)
		entity_index_patch:assert_called_once()
		entity_tag_patch:assert_called_once()
		Container.assert_equal(entity_tag_patch.calls[1], {test_world, entity_id, {tag}, entity})
		entity_untag_patch:assert_not_called()
		entity_index_patch:reset()
		entity_tag_patch:reset()
		Container.assert_equal({tags = {[tag] = true}}, entity_world:find(entity_id))
		Container.assert_equal({entity_world._entity_index:get_tags(entity_id)}, {1})

		entity.tags[tag] = nil
		entity_world:index(entity_id)
		entity_index_patch:assert_called_once()
		entity_tag_patch:assert_not_called()
		entity_untag_patch:assert_called_once()
		Container.assert_equal(entity_untag_patch.calls[1], {test_world, entity_id, {tag}, entity})
		entity_index_patch:reset()
		entity_untag_patch:reset()
		Container.assert_equal({entity_world._entity_index:get_tags(entity_id)}, {})

		entity_world:destroy(entity_id)
		Container.assert_equal(nil, entity_world:find(entity_id))
		entity_index_patch:assert_called_once()
		entity_tag_patch:assert_not_called()
		entity_untag_patch:assert_not_called()
	end,
	index_bounds = function()
		local world = World.World.new()
		local entity_world = world:require(Entity.WorldSys)
		world:start()

		local entity_id = entity_world:add{}
		local entity = entity_world:find(entity_id)
		Container.assert_equal({entity_world._entity_index:get_bounds(entity_id)}, {0, 0, 0, 0})

		local bounds_to_test = {
			{0, 0, 0, 0},
			{0, 0, 0, 1},
			{0, 0, 1, 0},
			{0, 0, 1, 1},
			{-1, -1, -1, -1},
			{0, 0, 32, 32},
			{-16, 32, 32, 64},
			{2^16, -2^16, 2^16 + 1, 2^16 + 1},
		}
		for _, bounds in ipairs(bounds_to_test) do
			entity.x, entity.y = bounds[1], bounds[2]
			entity.width, entity.height = bounds[3] - bounds[1], bounds[4] - bounds[2]
			entity_world:index(entity_id)
			Container.assert_equal({entity_world._entity_index:get_bounds(entity_id)}, bounds)
		end
	end,
	tag_is_set = function()
		local world = World.World.new()
		local entity_world = world:require(Entity.WorldSys)
		world:start()

		local entity_id = entity_world:add{}
		local entity = entity_world:find(entity_id)
		local tag = "t"
		local tag2 = "u"
		assert(not entity_world:tag_is_set(entity_id, {tag}))
		assert(not entity_world:tag_is_set(entity_id, {tag, tag2}))
		assert(not entity_world:tag_is_set(entity_id, {tag2}))

		entity.tags = {t = true}
		entity_world:index(entity_id, entity)
		assert(entity_world:tag_is_set(entity_id, {tag}))

		entity.tags[tag2] = true
		entity_world:index(entity_id, entity)
		assert(entity_world:tag_is_set(entity_id, {tag, tag2}))
	end,
	tag_untag = function()
		local world = World.World.new()
		local entity_world = world:require(Entity.WorldSys)
		entity_world:bounds_index_tag("yes")
		entity_world:bounds_index_tag("no")

		world:start()

		local entity_id, entity = entity_world:add{}
		Container.assert_equal(entity.tags, nil)
		Container.assert_equal(entity_world:tag_get_all("yes"), {})
		Container.assert_equal(entity_world:tag_get_all("no"), {})
		Container.assert_equal({entity_world._entity_index:get_tags(entity_id)}, {})

		entity_world:tag(entity_id, {"yes", "no"}, entity)
		Container.assert_equal(entity.tags, {yes = true, no = true})
		Container.assert_equal(entity_world:tag_get_all("yes"), {entity})
		Container.assert_equal(entity_world:tag_get_all("no"), {entity})
		Container.assert_equal({entity_world._entity_index:get_tags(entity_id)}, {1, 2})

		entity_world:untag(entity_id, {"no"}, entity)
		Container.assert_equal(entity.tags, {yes = true})
		Container.assert_equal(entity_world:tag_get_all("yes"), {entity})
		Container.assert_equal(entity_world:tag_get_all("no"), {})
		Container.assert_equal({entity_world._entity_index:get_tags(entity_id)}, {1})

		entity_world:untag(entity_id, {"yes", "no"}, entity)
		Container.assert_equal(entity.tags, {})
		Container.assert_equal(entity_world:tag_get_all("yes"), {})
		Container.assert_equal(entity_world:tag_get_all("no"), {})
		Container.assert_equal({entity_world._entity_index:get_tags(entity_id)}, {})
	end,
	bounds_set_get = function()
		local world = World.World.new()
		local entity_world = world:require(Entity.WorldSys)
		world:start()
		local entity_id, entity = entity_world:add{}
		Container.assert_equal({entity_world:bounds_get(entity_id, entity)}, {0, 0, 0, 0})

		local bounds_to_test = {
			{0, 0, 0, 0},
			{0, 0, 0, 1},
			{0, 0, 1, 0},
			{0, 0, 1, 1},
			{-1, -1, 0, 0},
			{0, 0, 32, 32},
			{-16, 32, 32, 64},
			{2^16, -2^16, 2^16 + 1, 2^16 + 1},
		}
		for _, bounds in ipairs(bounds_to_test) do
			entity_world:bounds_set(entity_id, Shim.unpack(bounds))
			Container.assert_equal({entity_world:bounds_get(entity_id)}, bounds)
		end
	end,
	run_world = function()
		local world = World.World.new()
		local entity_world = world:require(Entity.WorldSys)
		world:start()

		local entity_id = entity_world:add{x = 0, y = 0, width = 8, height = 8, tags = {hello = true}}
		entity_world:index(entity_id)
		entity_world:set(entity_id, {width = 16, height = 32})
		entity_world:set(entity_id, {tags = {world = true}})
		entity_world:set(entity_id, {tags = {hello = true, world = true}})
		entity_world:destroy(entity_id)
		entity_world:index(entity_id)

		Testing.assert_fails(function()
			entity_world:set(entity_id, {width = 16, height = 32})
		end)

		local empty_entity_id = entity_world:add{}
		entity_world:index(empty_entity_id)
		entity_world:destroy(empty_entity_id)

		local target_entity_count = 10
		for i = 1, target_entity_count do
			entity_world:add{i = i, tags = {["tag"..i] = true}}
		end

		for i, entity_inner in ipairs(entity_world:get_all_raw()) do
			assert(not entity_inner.destroyed)
			assert(entity_inner.i == i)
		end

		local target_destroy_count = 2
		for i = 1, target_destroy_count do
			entity_world:destroy(i)
		end

		local destroyed_count = 0
		local exists_count = 0
		for i, entity_inner in ipairs(entity_world:get_all_raw()) do
			if entity_inner.destroyed then
				destroyed_count = destroyed_count + 1
				assert(entity_inner.i == nil)
			else
				exists_count = exists_count + 1
				assert(entity_inner.i == i)
			end
		end
		assert(destroyed_count == target_destroy_count)
		assert(exists_count == (target_entity_count - target_destroy_count))

		assert(entity_world:get_max_id() == target_entity_count)

		world:step()
		world:stop()
		world:finalize()
	end,
	run_world_initial_state = function()
		local state = {entity = {entities = {
			{name = "hello", x = 0, y = 0, width = 8, height = 8, tags = {yes = true}},
			{name = "world"},
		}}}
		local world = World.World.new(nil, state)
		local entity_world = world:require(Entity.WorldSys)
		world:start()

		assert(entity_world:find(1).name == "hello")
		assert(entity_world:find(2).name == "world")
		assert(entity_world:find(3) == nil)

		world:step()
		world:stop()
		world:finalize()
	end,
	bounds_index_tag_too_many_tags = function()
		local world = World.World.new()
		local entity_world = world:require(Entity.WorldSys)

		for i = 1, Entity.Entity.max_tag_id do
			entity_world:bounds_index_tag("tag"..i)
		end

		Testing.assert_fails(function()
			entity_world:bounds_index_tag("one_too_many")
		end)
	end,
})

return Entity
