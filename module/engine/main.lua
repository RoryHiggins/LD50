local function require_entity_module()
local destroyed_entity = {
	exists = false,
}

local Sys = {}
Sys.__index = Sys
Sys.key = "Entity"
function Sys:create()
	local world = self.world
	local destroyed_entity_ids = world.destroyed_entity_ids
	local entities = world.entities
	local entity_id = #entities

	local destroyed_entity_ids_count = #destroyed_entity_ids
	if destroyed_entity_ids_count > 0 then
		entity_id = destroyed_entity_ids[destroyed_entity_ids_count]
		destroyed_entity_ids[destroyed_entity_ids_count] = nil
	end

	local entity = {
		id = entity_id,
		exists = true,
	}
	entities[entity_id] = entity

	return entity
end
function Sys:destroy(entity)
	if not entity.exists then
		return
	end

	local world = self.world
	local destroyed_entity_ids = world.destroyed_entity_ids
	local entities = world.entities
	local entity_id = entity.id

	entity.exists = false
	entities[entity_id] = destroyed_entity
	destroyed_entity_ids[#destroyed_entity_ids] = entity_id
end
function Sys:on_world_init(world)
	world.entities = {}
	world.destroyed_entity_ids = {}

	self.world = world
end
function Sys:on_init(simulation)
	self.simulation = simulation
	self:on_world_init(simulation.world)
end

return Sys
end


local function main()
	local Entity = require_entity_module()

	local simulation = {world = {}}
	local entity_sys = setmetatable({}, Entity)
	entity_sys:on_init(simulation)

	local entity = entity_sys:create()
	entity_sys:destroy(entity)
end

main()
