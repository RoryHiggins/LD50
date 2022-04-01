local Engine = require("engine/engine")
local ExampleSystems = require("examples/template/systems")

local Example = {}
Example.GameSys = Engine.Game.Sys.new_metatable("example")

local debug_checks_enabled = Engine.Core.Debugging.debug_checks_enabled

local function main()
	local state = {
		client = {width = 1024, height = 768},
		world = {client = {width = 128, height = 96}},
	}

	local game_sim = Engine.Game.Game.new(state)
	for _, SysModule in pairs(Engine.Systems) do
		if type(SysModule) == "table" and SysModule.GameSys ~= nil then
			game_sim:require(SysModule.GameSys)
		end
	end
	for _, SysModule in pairs(ExampleSystems) do
		if type(SysModule) == "table" and SysModule.GameSys ~= nil then
			game_sim:require(SysModule.GameSys)
		end
	end

	game_sim:require(Example.GameSys)
	game_sim:run()

	if debug_checks_enabled then
		game_sim._world:save("world.save.json")
		game_sim:save("game.save.json")
	end
end

main()
