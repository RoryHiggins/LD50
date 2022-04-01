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

	local game_save = "game.save.json"
	local game = Engine.Game.Game.new(state)
	game:load(game_save)

	for _, SysModule in pairs(Engine.Systems) do
		if type(SysModule) == "table" and SysModule.GameSys ~= nil then
			game:require(SysModule.GameSys)
		end
	end
	for _, SysModule in pairs(ExampleSystems) do
		if type(SysModule) == "table" and SysModule.GameSys ~= nil then
			game:require(SysModule.GameSys)
		end
	end

	game:require(Example.GameSys)
	game:run()

	game:save("game.save.json")

	if debug_checks_enabled then
		game._world:save("world.save.json")
	end
end

main()
