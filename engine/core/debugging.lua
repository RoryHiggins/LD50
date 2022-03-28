local Shim = require("engine/core/shim")
local Logging = require("engine/core/logging")

local debugger_lib = nil

local function noop()
end

-- set global breakpoint function for throaway debug breakpoints
breakpoint = noop

local Debugging = {}
Debugging.debugger_enabled = false
Debugging.debug_checks_enabled = false
Debugging.expensive_debug_checks_enabled = false
Debugging.breakpoint = breakpoint
function Debugging.set_debugger_enabled(enabled)
	Debugging.debugger_enabled = enabled

	breakpoint = noop
	Debugging.breakpoint = noop

	if Debugging.debugger_enabled then
		if debugger_lib == nil then
			debugger_lib = require("engine/lib/debugger/debugger")
		end

		breakpoint = debugger_lib
		Debugging.breakpoint = debugger_lib
	end

	return Debugging
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
