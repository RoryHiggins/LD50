local schema = require("engine/core/schema")
local sim = require("engine/engine/sim")
local testing = require("engine/core/testing")

local Sys = {}
setmetatable(Sys, sim.Sys)
Sys.__index = Sys
Sys.schema = schema.AllOf(sim.Sys.schema, schema.PartialObject{_is_game_sys = schema.Const(true)})
Sys.metatable_schema = schema.AllOf(sim.Sys.metatable_schema, schema.PartialObject{_is_game_sys = schema.Const(true)})
Sys._is_game_sys = true
function Sys.new_metatable(sys_name, metatable)
	assert(schema.Optional(Sys.metatable_schema)(metatable))
	return sim.Sys.new_metatable(sys_name, metatable or Sys)
end

local Game = {}
setmetatable(Game, sim.Sim)
Game.__index = Game
Game._is_game_sim = true
Game.schema = schema.AllOf(sim.Sim.schema, schema.PartialObject{_is_game_sim = schema.Const(true)})
Game.metatable_schema = schema.AllOf(sim.Sim.metatable_schema, schema.PartialObject{_is_game_sim = schema.Const(true)})
Game.Sys = Sys
function Game.new(state, settings, metatable)
	assert(schema.Optional(Game.metatable_schema)(metatable))
	return sim.Sim.new(state, settings, metatable or Game)
end

local tests = testing.add_suite("engine.game", {
	new = function()
		local game = Game.new()

		local TestSys = Sys.new_metatable("game_test")
		game:require(TestSys)
		game:set_status(sim.model.Status.running)
		game:set_status(sim.model.Status.stopped)
	end
})

local game = {}
game.Sys = Sys
game.Game = Game
game.tests = tests

return game
