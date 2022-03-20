local debugging = require("engine/core/debugging")
local schema = require("engine/core/schema")
local container = require("engine/core/container")
local testing = require("engine/core/testing")
local sim = require("engine/engine/sim")
local game = require("engine/engine/game")

local debug_checks_enabled = debugging.debug_checks_enabled

local Sys = {}
setmetatable(Sys, sim.Sys)
Sys.__index = Sys
Sys.schema = schema.AllOf(sim.Sys.schema, schema.PartialObject{_is_world_sys = schema.Const(true)})
Sys.metatable_schema =
	schema.AllOf(sim.Sys.metatable_schema, schema.PartialObject{_is_world_sys = schema.Const(true)})
Sys._is_world_sys = true
function Sys.new_metatable(sys_name, metatable)
	assert(schema.LabelString(sys_name))
	assert(schema.Optional(Sys.metatable_schema)(metatable))
	return sim.Sys.new_metatable(sys_name, metatable or Sys)
end

local World = {}
setmetatable(World, sim.Sim)
World.__index = World
World._is_world_sim = true
World.schema = schema.AllOf(sim.Sim.schema, schema.PartialObject{_is_world_sim = schema.Const(true)})
World.metatable_schema =
	schema.AllOf(sim.Sim.metatable_schema, schema.PartialObject{_is_world_sim = schema.Const(true)})
World.Sys = Sys
function World.new(state, settings, metatable)
	assert(schema.Optional(schema.SerializableObject)(state))
	assert(schema.Optional(schema.SerializableObject)(settings))
	assert(schema.Optional(World.metatable_schema)(metatable))
	return sim.Sim.new(state, settings, metatable or World)
end

local GameSys = game.Sys.new_metatable("world")
function GameSys:on_start()
	if debug_checks_enabled then
		assert(GameSys.schema(self))
		assert(self.sim.status == sim.model.Status.started)
	end

	container.object_set_defaults(self.state, {
		initial_world = {},
	})
	self:set(self:new_world(self.state.initial_world))
end
function GameSys:new_world(state, settings)
	if debug_checks_enabled then
		assert(GameSys.schema(self))
		assert(self.sim.status == sim.model.Status.started)
	end

	state = state or {}
	settings = settings or self.settings
	assert(schema.SerializableObject(state))
	assert(schema.SerializableObject(settings))

	local world = World.new(state, settings)

	return world
end
function GameSys:reset()
	if debug_checks_enabled then
		assert(GameSys.schema(self))
		assert(self.sim.status == sim.model.Status.started)
	end

	if self.world == nil then
		return
	end

	-- if this is new, we likely have an infinite loop of world creation
	assert(self.world.status ~= sim.model.Status.new)

	self.sim:broadcast("on_world_finalize", self.world)
	self.world:finalize()
	self.world = nil

	if debug_checks_enabled then
		assert(GameSys.schema(self))
	end
end
function GameSys:set(world)
	if debug_checks_enabled then
		assert(GameSys.schema(self))
		assert(World.schema(world))
		assert(self.sim.status == sim.model.Status.started)
		assert(world.status == sim.model.Status.new)
	end

	self:reset()

	self.world = world
	self.sim:broadcast("on_world_init")

	self.world:start()
	self.sim:broadcast("on_world_start")

	if debug_checks_enabled then
		assert(GameSys.schema(self))
	end
end

local tests = testing.add_suite("engine.world", {
	run_world = function()
		local TestSys = Sys.new_metatable("test")
		local world = World.new()
		world:require(TestSys)
		world:start()
		world:finalize()
	end,
	run_game = function()
		local game_ = game.Game.new()

		local world_game = game_:require(GameSys)
		game_:start()
		world_game:set(world_game:new_world())
		game_:finalize()
	end
})

local world = {}
world.Sys = Sys
world.World = World
world.GameSys = GameSys
world.tests = tests

return world
