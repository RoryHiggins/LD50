local Debugging = require("engine/core/debugging")
local Schema = require("engine/core/Schema")
local Container = require("engine/core/container")
local Testing = require("engine/core/testing")
local Sim = require("engine/engine/sim")
local Game = require("engine/engine/game")

local debug_checks_enabled = Debugging.debug_checks_enabled

local World = {}

World.Sys = {}
setmetatable(World.Sys, Sim.Sys)
World.Sys.__index = World.Sys
World.Sys.Schema = Schema.AllOf(
	Sim.Sys.Schema, Schema.PartialObject{_is_world_sys = Schema.Optional(Schema.Const(true))})
World.Sys.metatable_schema = Schema.AllOf(
	Sim.Sys.metatable_schema, Schema.PartialObject{_is_world_sys = Schema.Optional(Schema.Const(true))})
World.Sys._is_world_sys = true
function World.Sys.new_metatable(sys_name, metatable)
	assert(Schema.LabelString(sys_name))
	assert(Schema.Optional(World.Sys.metatable_schema)(metatable))
	return Sim.Sys.new_metatable(sys_name, metatable or World.Sys)
end

World.World = {}
setmetatable(World.World, Sim.Sim)
World.World.__index = World.World
World.World._is_world_sim = true
World.World.Schema = Schema.AllOf(Sim.Sim.Schema, Schema.PartialObject{_is_world_sim = Schema.Const(true)})
World.World.metatable_schema =
	Schema.AllOf(Sim.Sim.metatable_schema, Schema.PartialObject{_is_world_sim = Schema.Const(true)})
World.World.Sys = World.Sys
function World.World.new(state, settings, metatable)
	assert(Schema.Optional(Schema.SerializableObject)(state))
	assert(Schema.Optional(Schema.SerializableObject)(settings))
	assert(Schema.Optional(World.World.metatable_schema)(metatable))
	return Sim.Sim.new(state, settings, metatable or World.World)
end

World.GameSys = Game.Sys.new_metatable("world")
function World.GameSys:on_start()
	if debug_checks_enabled then
		assert(World.GameSys.Schema(self))
		assert(self.sim.status == Sim.Model.Status.started)
	end

	Container.set_defaults(self.state, {
		initial_world = {},
	})
	self:set(self:new_world(self.state.initial_world))
end
function World.GameSys:new_world(state, settings)
	if debug_checks_enabled then
		assert(World.GameSys.Schema(self))
		assert(self.sim.status == Sim.Model.Status.started)
	end

	state = state or {}
	settings = settings or self.settings
	assert(Schema.SerializableObject(state))
	assert(Schema.SerializableObject(settings))

	local world_sim = World.World.new(state, settings)

	return world_sim
end
function World.GameSys:reset()
	if debug_checks_enabled then
		assert(World.GameSys.Schema(self))
		assert(self.sim.status == Sim.Model.Status.started)
	end

	if self.world == nil then
		return
	end

	-- if this is new, we likely have an infinite loop of World creation
	assert(self.world.status ~= Sim.Model.Status.new)

	self.sim:broadcast("on_world_finalize", self.world)
	self.world:finalize()
	self.world = nil

	if debug_checks_enabled then
		assert(World.GameSys.Schema(self))
	end
end
function World.GameSys:set(world_)
	if debug_checks_enabled then
		assert(World.GameSys.Schema(self))
		assert(World.World.Schema(world_))
		assert(self.sim.status == Sim.Model.Status.started)
		assert(world_.status == Sim.Model.Status.new)
	end

	self:reset()

	self.world = world_
	self.sim:broadcast("on_world_init")

	self.world:start()
	self.sim:broadcast("on_world_start")

	if debug_checks_enabled then
		assert(World.GameSys.Schema(self))
	end
end

World.tests = Testing.add_suite("engine.world", {
	run_world = function()
		local TestSys = World.Sys.new_metatable("test")
		local world_sim = World.World.new()
		world_sim:require(TestSys)
		world_sim:start()
		world_sim:step()
		world_sim:finalize()
	end,
	run_game = function()
		local game_sim = Game.Game.new()
		local world_game = game_sim:require(World.GameSys)
		game_sim:start()
		game_sim:step()
		world_game:set(world_game:new_world())
		game_sim:step()
		game_sim:finalize()
	end
})

return World
