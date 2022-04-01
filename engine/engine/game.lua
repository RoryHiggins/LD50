local Debugging = require("engine/core/debugging")
local Schema = require("engine/core/schema")
local Testing = require("engine/core/testing")
local Sim = require("engine/engine/sim")

local debug_checks_enabled = Debugging.debug_checks_enabled
local expensive_debug_checks_enabled = Debugging.expensive_debug_checks_enabled

local Game = {}

Game.Sys = {}
setmetatable(Game.Sys, Sim.Sys)
Game.Sys.__index = Game.Sys
Game.Sys.Schema = Schema.AllOf(Sim.Sys.Schema, Schema.PartialObject{
	_is_game_sys = Schema.Optional(Schema.Const(true)),
	_is_world_sys = Schema.Optional(Schema.Const(false)),
})
Game.Sys.MetatableSchema = Schema.AllOf(Sim.Sys.MetatableSchema, Schema.PartialObject{
	_is_game_sys = Schema.Optional(Schema.Const(true)),
	_is_world_sys = Schema.Optional(Schema.Const(false)),
})
Game.Sys._is_game_sys = true
Game.Sys._is_world_sys = false
function Game.Sys.new_metatable(sys_name, metatable)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Schema.Optional(Game.Sys.MetatableSchema)(metatable))
		end
		assert(Schema.LabelString(sys_name))
	end

	return Sim.Sys.new_metatable(sys_name, metatable or Game.Sys)
end

Game.Game = {}
setmetatable(Game.Game, Sim.Sim)
Game.Game.__index = Game.Game
Game.Game._is_game_sim = true
Game.Game.Schema = Schema.AllOf(Sim.Sim.Schema, Schema.PartialObject{
	_is_game_sim = Schema.Const(true)
})
Game.Game.MetatableSchema = Schema.AllOf(
	Sim.Sim.MetatableSchema, Schema.PartialObject{_is_game_sim = Schema.Const(true)})
Game.Game.Sys = Game.Sys
function Game.Game.new(state, metatable)
	if expensive_debug_checks_enabled then
		assert(Schema.Optional(Schema.SerializableObject)(state))
		assert(Schema.Optional(Game.Game.MetatableSchema)(metatable))
	end

	return Sim.Sim.new(state, metatable or Game.Game)
end

Game.tests = Testing.add_suite("engine.game", {
	game_loop = function()
		local game_sim = Game.Game.new()

		local TestSys = Game.Sys.new_metatable("test")
		game_sim:require(TestSys)
		game_sim:start()
		game_sim:step()
		game_sim:finalize()
	end
})

return Game
