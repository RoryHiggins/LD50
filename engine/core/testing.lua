local logging = require("engine/core/logging")
local debugging = require("engine/core/debugging")

local CallWatcher = {}
CallWatcher.__index = CallWatcher

function CallWatcher:reset()
	self.calls = {}
end
function CallWatcher:assert_not_called()
	assert(#self.calls == 0, "must not be called")
end
function CallWatcher:assert_called()
	assert(#self.calls > 0, "must be called at least once")
end
function CallWatcher:assert_called_once()
	assert(#self.calls == 1, "must be called exactly once")
end
function CallWatcher.create(fn)
	assert(
		type(fn) == "function" or type(fn) == "nil",
		"fn must be a function or nil")
	if fn == nil then
		fn = function() end
	end

	local watcher = {
		calls = {},
	}
	setmetatable(watcher, CallWatcher)
	function watcher.call (...)
		watcher.calls[#watcher.calls + 1] = {...}
		return fn(...)
	end

	return watcher
end
function CallWatcher.patch(xs, key, fn)
	assert(type(xs) == "table", "xs must be a table")

	if fn == nil then
		fn = xs[key]
	end
	local watcher = CallWatcher.create(fn)
	xs[key] = watcher.call
	return watcher
end

local Test = {}
Test.__index = Test
function Test:run()
	logging.debug("Running test %s.%s", self.suite_name, self.name)

	local ok, err = debugging.pcall(self.fn)
	if not ok then
		logging.error("Failed test %s.%s, err=\n%s", self.suite_name, self.name, err)
		return false
	end

	logging.debug("Completed test %s.%s", self.suite_name, self.name)
	return true
end
function Test.create(suite_name, name, fn)
	assert(type(suite_name) == "string", "suite_name must be a string")
	assert(type(name) == "string", "name must be a string")
	assert(type(fn) == "function", "fn must be a function")

	local test = {
		_is_test_instance = true,
		suite_name = suite_name,
		name = name,
		fn = fn,
	}
	setmetatable(test, Test)
	return test
end

local TestSuite = {}
TestSuite.__index = TestSuite
function TestSuite:add(test)
	assert(test._is_test_instance, "test must be a test")

	self.tests[#self.tests + 1] = test
end
function TestSuite.create(name)
	assert(type(name) == "string", "name must be a string")

	local suite = {
		_is_test_suite_instance = true,
		name = name,
		tests = {},
	}
	setmetatable(suite, TestSuite)
	return suite
end
function TestSuite:run()
	local ok = true
	for _, test in ipairs(self.tests) do
		if not test:run() then
			ok = false
		end
	end

	return ok
end

local testing = {}
testing.CallWatcher = CallWatcher
testing.test_suites = {}
function testing.assert_fails(fn, ...)
	local debugger_enabled = debugging.debugger_enabled
	if debugger_enabled then
		debugging.set_debugger_enabled(false)
	end
	local result = pcall(fn, ...)
	if debugger_enabled then
		debugging.set_debugger_enabled(true)
	end
	assert(not result, "fn must raise an error")
end
function testing.add_suite(suite_name, test_name_fn_map)
	assert(type(suite_name) == "string", "suite_name must be a string")
	assert(type(test_name_fn_map) == "table", "test_name_fn_map must be a table")

	local test_suite = TestSuite.create(suite_name)
	for test_name, test_fn in pairs(test_name_fn_map) do
		assert(
			type(test_name) == "string",
			"test_name_fn_map must be a string: function mapping")
		assert(
			type(test_fn) == "function",
			"test_name_fn_map must be a string: function mapping")

		test_suite:add(Test.create(suite_name, test_name, test_fn))
	end
	testing.test_suites[#testing.test_suites + 1] = test_suite
	return test_suite
end
function testing.run_all()
	logging.info("Running all lua tests")
	local count = 0
	local ok = true
	for _, test_suite in ipairs(testing.test_suites) do
		if not test_suite:run() then
			ok = false
		end
		count = count + #test_suite.tests
	end

	if not ok then
		logging.info("Not all lua tests completed successfully")
		return false
	end

	logging.info("Completed %s lua tests", count)
	return true
end
function testing.suppress_errors(fn)
	local debugger_enabled = debugging.debugger_enabled
	if debugger_enabled then
		debugging.set_debugger_enabled(false)
	end

	logging.push_level(logging.Level.none)
	fn()

	if debugger_enabled then
		debugging.set_debugger_enabled(true)
	end

	assert(logging.pop_level() == true)
end

return testing
