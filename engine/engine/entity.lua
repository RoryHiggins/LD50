local Shim = require("engine/core/shim")
local Math = require("engine/core/math")
local Debugging = require("engine/core/debugging")
local Logging = require("engine/core/logging")
local Testing = require("engine/core/testing")
local Schema = require("engine/core/Schema")
local Container = require("engine/core/container")
local Client = require("engine/engine/client")
local World = require("engine/engine/world")

local debug_checks_enabled = Debugging.debug_checks_enabled
local expensive_debug_checks_enabled = Debugging.expensive_debug_checks_enabled

local Entity = {}

Entity.Tags = {}
Entity.Tags.max_id = Client.Wrappers.EntityIndex.get_max_tag_id() - 1  -- -1 for lua array indexing
Entity.Tags.Schema = Schema.Mapping(Schema.LabelString, Schema.Const(true))
Entity.Tags.defaults = {}

Entity.Bounds = {}
Entity.Bounds.Schema = Schema.Object{
	x = Schema.Optional(Schema.Integer),
	y = Schema.Optional(Schema.Integer),
	width = Schema.Optional(Schema.NonNegativeInteger),
	height = Schema.Optional(Schema.NonNegativeInteger),
}
Entity.Bounds.defaults = {
	x = 0,
	y = 0,
	width = 0,
	height = 0,
}

Entity.Sprite = {}
Entity.Sprite.Schema = Schema.Object{
	sprite_name = Schema.LabelString,
	z = Schema.Optional(Schema.Number),
	r = Schema.Optional(Schema.BoundedInteger(0, 255)),
	g = Schema.Optional(Schema.BoundedInteger(0, 255)),
	b = Schema.Optional(Schema.BoundedInteger(0, 255)),
	a = Schema.Optional(Schema.BoundedInteger(0, 255)),
	translate_x = Schema.Optional(Schema.Number),
	translate_y = Schema.Optional(Schema.Number),
	scale_x = Schema.Optional(Schema.BoundedNumber(Math.epsilon, Math.integer_max)),
	scale_y = Schema.Optional(Schema.BoundedNumber(Math.epsilon, Math.integer_max)),
}
Entity.Sprite.defaults = {
	sprite_name = "none",  -- TODO update once sprite sys in place
}

Entity.Entity = {}
Entity.Entity.DestroyedSchema = Schema.Object{
	destroyed = Schema.Const(true),
}
Entity.Entity.ExistsSchema = Schema.AllOf(Schema.SerializableObject, Schema.PartialObject{
	destroyed = Schema.Nil,
	name = Schema.Optional(Schema.LabelString),
	tags = Schema.Optional(Entity.Tags.Schema),
	bounds = Schema.Optional(Entity.Bounds.Schema),
	sprite = Schema.Optional(Entity.Sprite.Schema),
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
	_entity_index = Client.Wrappers.Schema("EntityIndex"),
	_entity_ids_free = Schema.Array(Schema.PositiveInteger),
	_entity_to_entity_id = Schema.Mapping(Entity.Entity.Schema, Schema.PositiveInteger),
	_tag_id_to_tag = Schema.BoundedArray(Schema.LabelString, 0, Entity.Tags.max_id),
	_tag_to_tag_id = Schema.Mapping(Schema.LabelString, Schema.BoundedInteger(0, Entity.Tags.max_id)),
	_tag_to_entities = Schema.Mapping(Schema.LabelString, Schema.Array(Entity.Entity.Schema)),
	_entity_id_to_tag_indices = (  -- for efficient cleanup of _tag_to_entities
		Schema.Mapping(Schema.PositiveInteger, Schema.Mapping(Schema.LabelString, Schema.PositiveInteger))),
	-- _sprite_game = Optional(Sprite.GameSys.Schema),  -- TODO once sprite sys is integrated
})
function Entity.WorldSys:add(state)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.SerializableObject(state))
		assert(not state.destroyed)
	end

	local entities = self.state.entities
	local entity_id = #entities + 1

	local entity_ids_free = self._entity_ids_free
	if #entity_ids_free > 0 then
		entity_id = entity_ids_free[#entity_ids_free]
		entity_ids_free[#entity_ids_free] = nil
	end

	local entity = entities[entity_id]
	if entity == nil then
		entity = {}
		entities[entity_id] = entity
	end
	entity.destroyed = nil

	entities[entity_id] = entity

	if state ~= nil then
		Container.update(entity, state)
	end

	self:_index_impl(entity_id, entity)

	if expensive_debug_checks_enabled then
		assert(Schema.PositiveInteger(entity_id))
		assert(Entity.WorldSys.Schema(self))
	end

	return entity_id
end
function Entity.WorldSys:set(entity_id, state)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
		assert(Schema.SerializableObject(state))
		assert(not state.destroyed)
	end

	local entity = self:find(entity_id)
	if entity == nil then
		error("entity not found")
		return
	end

	Container.update(entity, state)

	self:_index_impl(entity_id, entity)

	if expensive_debug_checks_enabled then
		assert(Entity.Entity.Schema(entity))
	end
end
function Entity.WorldSys:destroy(entity_id)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
	end

	local entity = self:find(entity_id)
	if entity == nil then
		error("entity not found")
		return
	end

	for key, _ in pairs(entity) do
		entity[key] = nil
	end
	entity.destroyed = true

	local entity_ids_free = self._entity_ids_free
	entity_ids_free[#entity_ids_free + 1] = entity_id

	self:_index_impl(entity_id, entity)

	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
	end
end
function Entity.WorldSys:index(entity_id)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
	end

	local entity = self:find(entity_id)
	if entity == nil then
		error("entity not found")
		return
	end

	self:_index_impl(entity_id, entity)

	if expensive_debug_checks_enabled then
		assert(Entity.Entity.Schema(entity))
	end
end
function Entity.WorldSys:find(entity_id)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.PositiveInteger(entity_id))
	end

	local entity = self.state.entities[entity_id]
	if entity == nil or entity.destroyed then
		return nil
	end

	return entity
end
function Entity.WorldSys:get_all_raw()
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
	end

	return self.state.entities
end
function Entity.WorldSys:get_max_id()
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
	end

	return #self.state.entities
end
function Entity.WorldSys:bounds_index_tag(tag, allow_failure)
	if expensive_debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
		assert(Schema.LabelString(tag))
		assert(Schema.Optional(Schema.Boolean)(allow_failure))
	end

	local tag_to_tag_id = self._tag_to_tag_id
	local tag_id_to_tag = self._tag_id_to_tag
	local tag_id = tag_to_tag_id[tag]
	if tag_id ~= nil then
		return tag_id
	end

	tag_id = #tag_id_to_tag + 1
	if tag_id > Entity.Tags.max_id then
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
function Entity.WorldSys:on_init()
	Container.set_defaults(self.state, Entity.WorldSys.State.defaults)

	self._entity_index = Client.Wrappers.EntityIndex.new{}
	self._entity_ids_free = {}
	self._entity_to_entity_id = {}
	self._tag_id_to_tag = {}
	self._tag_to_tag_id = {}
	self._tag_to_entities = {}
	self._entity_id_to_tag_indices = {}

	for entity_id, entity in ipairs(self.state.entities) do
		self:_index_impl(entity_id, entity)
	end

	if debug_checks_enabled then
		assert(Entity.WorldSys.Schema(self))
	end
end
function Entity.WorldSys:_index_impl(entity_id, entity)
	if entity.destroyed ~= true then
		self._entity_to_entity_id[entity] = entity_id
	end

	local entity_id_to_tag_indices = self._entity_id_to_tag_indices
	local tag_to_entities = self._tag_to_entities
	local entity_tag_indices = entity_id_to_tag_indices[entity_id]
	if entity_tag_indices == nil then
		entity_tag_indices = {}
		entity_id_to_tag_indices[entity_id] = entity_tag_indices
	end

	-- check for any removed tags
	local entity_tags = entity.tags or {}
	for tag, tag_index in pairs(entity_tag_indices) do
		if entity_tags[tag] == nil then
			local tag_entities = tag_to_entities[tag]

			local swap_tag_index = #tag_entities
			local swap_entity = tag_entities[swap_tag_index]
			local swap_entity_id = self._entity_to_entity_id[swap_entity]
			local swap_entity_tag_indices = entity_id_to_tag_indices[swap_entity_id]

			swap_entity_tag_indices[tag] = tag_index
			tag_entities[tag_index] = swap_entity
			tag_entities[swap_tag_index] = nil
			entity_tag_indices[tag] = nil
		end
	end

	local tag_to_tag_id = self._tag_to_tag_id
	local tag_ids = {}
	-- check for any added tags, and find all bounds-indexed tags
	for tag, value in pairs(entity_tags) do
		local tag_id = tag_to_tag_id[tag]
		if tag_id ~= nil then
			tag_ids[#tag_ids + 1] = tag_id
		end

		if value == true and entity_tag_indices[tag] == nil then
			local tag_entities = tag_to_entities[tag]
			if tag_entities == nil then
				tag_entities = {}
				tag_to_entities[tag] = tag_entities
			end

			local tag_index = #tag_entities + 1
			tag_entities[tag_index] = entity
			entity_tag_indices[tag] = tag_index
		end
	end

	local bounds = entity.bounds or Entity.Bounds.defaults
	local sprite = entity.sprite or Entity.Sprite.defaults
	local x1, y1 = bounds.x or 0, bounds.y or 0
	self._entity_index:set(
		entity_id,
		x1, y1, x1 + (bounds.width or 0), y1 + (bounds.height or 0),
		0, 0, 0, 0,  -- TODO extract UVs once sprite sys hooked up
		sprite.r or 255, sprite.g or 255, sprite.b or 255, sprite.a or 255,
		sprite.z or 0,
		-- transform; TODO extract once sprite sys hooked up
		sprite.scale_x or 1, 0, 0, 0,
		0, sprite.scale_y or 1, 0, 0,
		0, 0, 1, 0,
		sprite.translate_x or 0, sprite.translate_y or 0, 0, 1,
		Shim.unpack(tag_ids)
	)

	if entity.destroyed == true then
		self._entity_to_entity_id[entity] = nil
	end

end

Entity.tests = Testing.add_suite("engine.entity", {
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

		Testing.assert_fails(function()
			entity_world:index(entity_id)
		end)

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
		world:finalize()
	end,
	too_many_tags = function()
		local world = World.World.new()
		local entity_world = world:require(Entity.WorldSys)

		for i = 1, Entity.Tags.max_id do
			entity_world:bounds_index_tag("tag"..i)
		end

		Testing.assert_fails(function()
			entity_world:bounds_index_tag("one_too_many")
		end)
	end,
})
return Entity
