local Engine = require("engine/engine")
local ExampleSystems = require("ld50/systems")

local LD50 = {}
LD50.GameSys = Engine.Game.Sys.new_metatable("ld50")
LD50.GameSys.WorldSys = Engine.World.Sys.new_metatable("ld50")

local debug_checks_enabled = Engine.Core.Debugging.debug_checks_enabled

local function main()
	local state = {
		client = {width = 1024, height = 768},
		world = {client = {width = 128, height = 96}},
	}

	-- local game_save = "game.save.json"
	local game = Engine.Game.Game.new(state)
	-- game:load(game_save)

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

	game:require(LD50.GameSys)

	game.music = Engine.Client.Wrappers.Music.new_file{
		filename = './ld50/data/sea_ambient.ogg'
	}
	game.music:play{volume = 0.3, loop_forever = true}

	game:run()

	-- game:save(game_save)

	if debug_checks_enabled then
		game._world:save("world.save.json")
	end
end

main()
