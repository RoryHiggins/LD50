local debugger_lib = nil

local debugger_enabled = false
local debugging = {}
function debugging.set_debugger_enabled(enabled)
	debugger_enabled = enabled

	if debugger_enabled and debugger_lib == nil then
		debugger_lib = require("engine/lib/debugger/debugger")
	end

	return debugging
end
function debugging.breakpoint()
	if not debugger_enabled then
		return
	end

	debugger_lib()
end
function debugging.protected_call(fn, ...)
	local args = {...}

	if not debugger_enabled then
		local function fn_wrapped()
			fn(unpack(args))
		end
		local function on_error(err)
			print(debug.traceback("error :"..tostring(err), 2))
		end

		return xpcall(fn_wrapped, on_error)
	end

	return debugger_lib.call(fn, unpack(args))
end

return debugging
