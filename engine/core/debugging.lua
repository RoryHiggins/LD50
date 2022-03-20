local shim = require("engine/core/shim")
local logging = require("engine/core/logging")

local debugger_lib = nil

local debugging = {}
debugging.debugger_enabled = false
debugging.debug_checks_enabled = true
function debugging.set_debugger_enabled(enabled)
	debugging.debugger_enabled = enabled

	if debugging.debugger_enabled and debugger_lib == nil then
		debugger_lib = require("engine/lib/debugger/debugger")
	end

	return debugging
end
function debugging.breakpoint()
	if not debugging.debugger_enabled then
		return
	end

	debugger_lib()
end
function debugging.pcall(fn, ...)
	local args = {...}

	if not debugging.debugger_enabled then
		local function fn_wrapped()
			fn(shim.unpack(args))
		end

		return pcall(fn_wrapped)
	end

	return debugger_lib.call(fn, shim.unpack(args))
end
logging.add_error_handler(debugging.breakpoint)

return debugging
