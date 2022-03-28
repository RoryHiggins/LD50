local Logging = {}
Logging.Level = {
	none = 0,
	error = 2,
	warn = 3,
	info = 4,
	debug = 5,
	trace = 6,
}
Logging.level = Logging.Level.info
Logging.error_count = 0
Logging._level_stack = {}
Logging.error_handlers = {}
function Logging.add_error_handler(handler)
	Logging.error_handlers[#Logging.error_handlers + 1] = handler
end
function Logging._log_impl(level, prefix, format, ...)
	if Logging.level < level then
		return
	end

	local callee_info = debug.getinfo(3, "Sln")
	print(string.format(prefix..format,
		callee_info.short_src,
		callee_info.currentline,
		callee_info.name or "<unnamed>",
		...
	))

	if level <= Logging.Level.warn then
		print(debug.traceback("", 3))
		if level <= Logging.Level.error then
			Logging.error_count = Logging.error_count + 1
		end

		io.flush()

		for _, handler in ipairs(Logging.error_handlers) do
			handler()
		end
	end

	io.flush()
end
function Logging.error(format, ...)
	Logging._log_impl(Logging.Level.error, "[ERROR %s:%d] %s() ", format, ...)
end
function Logging.warning(format, ...)
	Logging._log_impl(Logging.Level.warn, "[WARN  %s:%d] %s() ", format, ...)
end
function Logging.info(format, ...)
	Logging._log_impl(Logging.Level.info, "[info  %s:%d] %s() ", format, ...)
end
function Logging.debug(format, ...)
	Logging._log_impl(Logging.Level.debug, "[debug %s:%d] %s() ", format, ...)
end
function Logging.trace(format, ...)
	Logging._log_impl(Logging.Level.trace, "[trace %s:%d] %s() ", format, ...)
end
function Logging.push_level(new_level)
	Logging._level_stack[#Logging._level_stack + 1] = Logging.level
	Logging.level = new_level
end
function Logging.pop_level()
	if #Logging._level_stack == 0 then
		print("Logging.pop_level failed, log level may not be valid")
		Logging.level = Logging.Level.info
		return false
	end

	Logging.level = Logging._level_stack[#Logging._level_stack]
	Logging._level_stack[#Logging._level_stack] = nil
	return true
end

return Logging
