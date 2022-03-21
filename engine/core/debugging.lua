local Shim = require("engine/core/shim")
local Logging = require("engine/core/logging")

local debugger_lib = nil

local Debugging = {}
Debugging.debugger_enabled = false
Debugging.debug_checks_enabled = true
function Debugging.set_debugger_enabled(enabled)
	Debugging.debugger_enabled = enabled

	if Debugging.debugger_enabled and debugger_lib == nil then
		debugger_lib = require("engine/lib/debugger/debugger")
	end

	return Debugging
end
function Debugging.breakpoint()
	if not Debugging.debugger_enabled then
		return
	end

	debugger_lib()
end
function Debugging.pcall(fn, ...)
	local args = {...}

	if not Debugging.debugger_enabled then
		local function fn_wrapped()
			fn(Shim.unpack(args))
		end

		return pcall(fn_wrapped)
	end

	return debugger_lib.call(fn, Shim.unpack(args))
end
Logging.add_error_handler(Debugging.breakpoint)

return Debugging
