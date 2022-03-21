local Logging = require("engine/core/logging")
local Debugging = require("engine/core/debugging")

local Testing = {}

Testing.CallWatcher = {}
Testing.CallWatcher.__index = Testing.CallWatcher
function Testing.CallWatcher:reset()
	self.calls = {}
end
function Testing.CallWatcher:assert_not_called()
	assert(#self.calls == 0, "must not be called")
end
function Testing.CallWatcher:assert_called()
	assert(#self.calls > 0, "must be called at least once")
end
function Testing.CallWatcher:assert_called_once()
	assert(#self.calls == 1, "must be called exactly once")
end
function Testing.CallWatcher.create(fn)
	assert(
		type(fn) == "function" or type(fn) == "nil",
		"fn must be a function or nil")
	if fn == nil then
		fn = function() end
	end

	local watcher = {
		calls = {},
	}
	setmetatable(watcher, Testing.CallWatcher)
	function watcher.call (...)
		watcher.calls[#watcher.calls + 1] = {...}
		return fn(...)
	end

	return watcher
end
function Testing.CallWatcher.patch(xs, key, fn)
	assert(type(xs) == "table", "xs must be a table")

	if fn == nil then
		fn = xs[key]
	end
	local watcher = Testing.CallWatcher.create(fn)
	xs[key] = watcher.call
	return watcher
end

Testing.Test = {}
Testing.Test.__index = Testing.Test
function Testing.Test:run()
	Logging.debug("Running test %s.%s", self.suite_name, self.name)

	local ok, err = Debugging.pcall(self.fn)
	if not ok then
		Logging.error("Failed test %s.%s, err=\n%s", self.suite_name, self.name, err)
		return false
	end

	Logging.debug("Completed test %s.%s", self.suite_name, self.name)
	return true
end
function Testing.Test.create(suite_name, name, fn)
	assert(type(suite_name) == "string", "suite_name must be a string")
	assert(type(name) == "string", "name must be a string")
	assert(type(fn) == "function", "fn must be a function")

	local test = {
		_is_test_instance = true,
		suite_name = suite_name,
		name = name,
		fn = fn,
	}
	setmetatable(test, Testing.Test)
	return test
end

Testing.TestSuite = {}
Testing.TestSuite.__index = Testing.TestSuite
function Testing.TestSuite:add(test)
	assert(test._is_test_instance, "test must be a test")

	self.tests[#self.tests + 1] = test
end
function Testing.TestSuite.create(name)
	assert(type(name) == "string", "name must be a string")

	local suite = {
		_is_test_suite_instance = true,
		name = name,
		tests = {},
	}
	setmetatable(suite, Testing.TestSuite)
	return suite
end
function Testing.TestSuite:run()
	local ok = true
	for _, test in ipairs(self.tests) do
		if not test:run() then
			ok = false
		end
	end

	return ok
end

Testing.test_suites = {}
function Testing.assert_fails(fn, ...)
	local debugger_enabled = Debugging.debugger_enabled
	if debugger_enabled then
		Debugging.set_debugger_enabled(false)
	end
	local result = pcall(fn, ...)
	if debugger_enabled then
		Debugging.set_debugger_enabled(true)
	end
	assert(not result, "fn must raise an error")
end
function Testing.add_suite(suite_name, test_name_fn_map)
	assert(type(suite_name) == "string", "suite_name must be a string")
	assert(type(test_name_fn_map) == "table", "test_name_fn_map must be a table")

	local test_suite = Testing.TestSuite.create(suite_name)
	for test_name, test_fn in pairs(test_name_fn_map) do
		assert(
			type(test_name) == "string",
			"test_name_fn_map must be a string: function mapping")
		assert(
			type(test_fn) == "function",
			"test_name_fn_map must be a string: function mapping")

		test_suite:add(Testing.Test.create(suite_name, test_name, test_fn))
	end
	Testing.test_suites[#Testing.test_suites + 1] = test_suite
	return test_suite
end
function Testing.run_all()
	Logging.info("Running all lua tests")
	local count = 0
	local ok = true
	for _, test_suite in ipairs(Testing.test_suites) do
		if not test_suite:run() then
			ok = false
		end
		count = count + #test_suite.tests
	end

	if not ok then
		Logging.info("Not all lua tests completed successfully")
		return false
	end

	Logging.info("Completed %s lua tests", count)
	return true
end
function Testing.suppress_errors(fn)
	local debugger_enabled = Debugging.debugger_enabled
	if debugger_enabled then
		Debugging.set_debugger_enabled(false)
	end

	Logging.push_level(Logging.Level.none)
	fn()

	if debugger_enabled then
		Debugging.set_debugger_enabled(true)
	end

	assert(Logging.pop_level() == true)
end

return Testing
