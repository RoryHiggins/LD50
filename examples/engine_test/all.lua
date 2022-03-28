local function main()
	require('engine/core/container')
	require('engine/core/debugging')
	require('engine/core/json')
	require('engine/core/logging')
	require('engine/core/math')
	require('engine/core/model')
	require('engine/core/schema')
	require('engine/core/serialization')
	require('engine/core/shim')
	require('engine/core/testing')
	require('engine/engine/camera')
	require('engine/engine/client')
	require('engine/engine/controller')
	require('engine/engine/entity')
	require('engine/engine/game')
	require('engine/engine/sim')
	require('engine/engine/world')
	require('engine/lib/debugger/debugger')
	require('engine/lib/json/json')
	require("engine/core/testing").run_all()
	require("engine/engine/entity")
end
local Debugging = require("engine/core/debugging")
Debugging.debug_checks_enabled = true
Debugging.expensive_debug_checks_enabled = true
Debugging.set_debugger_enabled(true)
require("engine/core/debugging").pcall(main)
