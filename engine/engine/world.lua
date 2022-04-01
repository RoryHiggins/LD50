local Debugging = require("engine/core/debugging")
local Schema = require("engine/core/schema")
local Container = require("engine/core/container")
local Testing = require("engine/core/testing")
local Sim = require("engine/engine/sim")
local Game = require("engine/engine/game")

local debug_checks_enabled = Debugging.debug_checks_enabled
local expensive_debug_checks_enabled = Debugging.expensive_debug_checks_enabled

local World = {}

World.Sys = {}
setmetatable(World.Sys, Sim.Sys)
World.Sys.__index = World.Sys
World.Sys.Schema = Schema.AllOf(Sim.Sys.Schema, Schema.PartialObject{
	_is_world_sys = Schema.Optional(Schema.Const(true)),
	_is_game_sys = Schema.Optional(Schema.Const(false)),
})
World.Sys.MetatableSchema = Schema.AllOf(Sim.Sys.MetatableSchema, Schema.PartialObject{
	_is_world_sys = Schema.Optional(Schema.Const(true)),
	_is_game_sys = Schema.Optional(Schema.Const(false)),
})
World.Sys._is_world_sys = true
World.Sys._is_game_sys = false
function World.Sys.new_metatable(sys_name, metatable)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Schema.Optional(World.Sys.MetatableSchema)(metatable))
		end
		assert(Schema.LabelString(sys_name))
	end
	return Sim.Sys.new_metatable(sys_name, metatable or World.Sys)
end

World.World = {}
setmetatable(World.World, Sim.Sim)
World.World.__index = World.World
World.World._is_world_sim = true
World.World.Schema = Schema.AllOf(Sim.Sim.Schema, Schema.PartialObject{
	_is_world_sim = Schema.Const(true)
})
World.World.MetatableSchema =
	Schema.AllOf(Sim.Sim.MetatableSchema, Schema.PartialObject{_is_world_sim = Schema.Const(true)})
World.World.Sys = World.Sys
function World.World.new(game, state, metatable)
	if expensive_debug_checks_enabled then
		assert(Schema.Optional(Schema.SerializableObject)(state))
		assert(Schema.Optional(World.World.MetatableSchema)(metatable))
		assert(Schema.Optional(Game.Game.Schema)(game))
	end

	local world = Sim.Sim.new(state, metatable or World.World)
	world._game = game

	return world
end

World.GameSys = Game.Sys.new_metatable("world")
World.GameSys.State = {}
World.GameSys.State.Schema = Schema.SerializableObject
World.GameSys.State.defaults = {}
World.GameSys.Schema = Schema.AllOf(Game.Sys.Schema, Schema.PartialObject{
	state = World.GameSys.State.Schema,
	_world_systems = Schema.Array(World.Sys.MetatableSchema),
})
function World.GameSys:new_world(state)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(World.GameSys.Schema(self))
			assert(Schema.Optional(Schema.SerializableObject)(state))
		end
		assert(self.sim.status == Sim.Status.started)
	end

	state = state or {}

	local world = World.World.new(self.sim, state)
	for _, sys_metatable in ipairs(self._world_systems) do
		world:require(sys_metatable)
	end

	return world
end
function World.GameSys:reset()
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(World.GameSys.Schema(self))
		end

		-- if this is new, we likely have an infinite loop of World creation
		assert(self.world == nil or self.world.status ~= Sim.Status.new)
	end

	if self.world == nil then
		return
	end

	if self.world.status ~= Sim.Status.finalized then
		self.sim:broadcast("on_world_finalize")
		self.world:finalize()
	end

	self.world = nil

	if expensive_debug_checks_enabled then
		assert(World.GameSys.Schema(self))
	end
end
function World.GameSys:set(world)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(World.GameSys.Schema(self))
			assert(World.World.Schema(world))
		end

		assert(self.sim.status == Sim.Status.started)
		assert(world.status == Sim.Status.new)
	end

	self:reset()

	self.world = world
	self.sim._world = world
	self.sim:broadcast("on_world_set")

	self.world:start()
	self.sim:broadcast("on_world_start")

	if expensive_debug_checks_enabled then
		assert(World.GameSys.Schema(self))
		assert(World.World.Schema(world))
	end
end
function World.GameSys:require_world_sys(sys_metatable)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(World.Sys.MetatableSchema(sys_metatable))
		end

		assert(self.sim.status == Sim.Status.new)
		assert(Container.array_try_find(self._world_systems, sys_metatable) == nil)
	end

	self._world_systems[#self._world_systems + 1] = sys_metatable
end
function World.GameSys:on_init()
	Container.set_defaults(self.state, World.GameSys.State.defaults)

	self._world_systems = {}

	if expensive_debug_checks_enabled then
		assert(World.GameSys.State.Schema(self.state))
	end
end
function World.GameSys:on_step()
	if self.world ~= nil then
		self.world:step()
	end
end
function World.GameSys:on_start()
	self:set(self:new_world(self.state))
end
function World.GameSys:on_finalize()
	self:reset()
end

World.tests = Testing.add_suite("engine.world", {
	run_world = function()
		local game = Game.Game.new()

		local TestSys = World.Sys.new_metatable("test")
		local world = World.World.new(game)
		world:require(TestSys)
		world:start()
		world:step()
		world:finalize()
	end,
	run_game = function()
		local game = Game.Game.new()
		local world_game = game:require(World.GameSys)
		game:start()
		game:step()
		world_game:set(world_game:new_world())
		game:step()
		game:finalize()
	end
})

return World
