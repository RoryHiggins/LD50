local logging = {}
logging.Level = {
	none = 0,
	error = 2,
	warn = 3,
	info = 4,
	debug = 5,
	trace = 6,
}
logging.level = logging.Level.info
logging.error_count = 0
logging._level_stack = {}
logging.error_handlers = {}
function logging.add_error_handler(handler)
	logging.error_handlers[#logging.error_handlers + 1] = handler
end
function logging._log_impl(level, prefix, format, ...)
	if logging.level < level then
		return
	end

	local callee_info = debug.getinfo(3, "Sln")
	print(string.format(prefix..format,
		callee_info.short_src,
		callee_info.currentline,
		callee_info.name or "<unnamed>",
		...
	))

	if level <= logging.Level.warn then
		print(debug.traceback("", 3))
		if level <= logging.Level.error then
			logging.error_count = logging.error_count + 1
		end

		io.flush()

		for _, handler in ipairs(logging.error_handlers) do
			handler()
		end
	end

	io.flush()
end
function logging.error(format, ...)
	logging._log_impl(logging.Level.error, "[ERROR %s:%d] %s() ", format, ...)
end
function logging.warn(format, ...)
	logging._log_impl(logging.Level.warn, "[WARN  %s:%d] %s() ", format, ...)
end
function logging.info(format, ...)
	logging._log_impl(logging.Level.info, "[info  %s:%d] %s() ", format, ...)
end
function logging.debug(format, ...)
	logging._log_impl(logging.Level.debug, "[debug %s:%d] %s() ", format, ...)
end
function logging.trace(format, ...)
	logging._log_impl(logging.Level.trace, "[trace %s:%d] %s() ", format, ...)
end
function logging.push_level(new_level)
	logging._level_stack[#logging._level_stack + 1] = logging.level
	logging.level = new_level
end
function logging.pop_level()
	if #logging._level_stack == 0 then
		print("logging.pop_level failed, log level may not be valid")
		logging.level = logging.Level.info
		return false
	end

	logging.level = logging._level_stack[#logging._level_stack]
	logging._level_stack[#logging._level_stack] = nil
	return true
end

return logging
