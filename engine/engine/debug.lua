local Debugging = require("engine/core/debugging")
local Testing = require("engine/core/testing")
local Schema = require("engine/core/schema")
local Container = require("engine/core/container")
local World = require("engine/engine/world")
local Game = require("engine/engine/game")
local Entity = require("engine/engine/entity")
local Client = require("engine/engine/client")
local Controller = require("engine/engine/controller")
local Image = require("engine/engine/image")
local Text = require("engine/engine/text")

local debug_checks_enabled = Debugging.debug_checks_enabled
local expensive_debug_checks_enabled = Debugging.expensive_debug_checks_enabled

local Debug = {}

Debug.GameSys = Game.Sys.new_metatable("debug")
Debug.GameSys.WorldSys = Debug.WorldSys
function Debug.GameSys:on_init()
	self._world_game = self.sim:require(World.GameSys)
end
function Debug.GameSys:on_step()
	local quicksave_filename = "quicksave_world.save.json"
	local context = self.sim._context
	if debug_checks_enabled and context ~= nil then
		local window = context.window
		if window:get_key_state("f1", true) then
			self._world_game:save(quicksave_filename)
		elseif window:get_key_state("f3", true) then
			self._world_game:load(quicksave_filename)
		elseif window:get_key_state("f2", true) then
			breakpoint()
		end
	end
end

return Debug
