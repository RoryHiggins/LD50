local schema = require("engine/core/schema")
local sim = require("engine/engine/sim")
local testing = require("engine/core/testing")

local Sys = {}
setmetatable(Sys, sim.Sys)
Sys.__index = Sys
Sys.schema = schema.AllOf(sim.Sys.schema, schema.PartialObject{_is_world_sys = schema.Const(true)})
Sys.metatable_schema = schema.AllOf(sim.Sys.metatable_schema, schema.PartialObject{_is_world_sys = schema.Const(true)})
Sys._is_world_sys = true
function Sys.new_metatable(sys_name, metatable)
	assert(schema.Optional(Sys.metatable_schema)(metatable))
	return sim.Sys.new_metatable(sys_name, metatable or Sys)
end

local World = {}
setmetatable(World, sim.Sim)
World.__index = World
World._is_world_sim = true
World.schema = schema.AllOf(sim.Sim.schema, schema.PartialObject{_is_world_sim = schema.Const(true)})
World.metatable_schema = schema.AllOf(sim.Sim.metatable_schema, schema.PartialObject{_is_world_sim = schema.Const(true)})
World.Sys = Sys
function World.new(state, settings, metatable)
	assert(schema.Optional(World.metatable_schema)(metatable))
	return sim.Sim.new(state, settings, metatable or World)
end

local tests = testing.add_suite("engine.world", {
	new = function()
		local world = World.new()

		local TestSys = Sys.new_metatable("world_test")
		world:require(TestSys)
		world:set_status(sim.model.Status.running)
		world:set_status(sim.model.Status.stopped)
	end
})

local world = {}
world.Sys = Sys
world.World = World
world.tests = tests

return world
