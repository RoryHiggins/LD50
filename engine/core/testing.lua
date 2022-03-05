local debugging = require("engine/core/debugging")
local logging = require("engine/core/logging")

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
	fn = fn or function() end

	local watcher = {
		calls = {},
	}
	setmetatable(watcher, CallWatcher)
	function watcher.call (...)
		fn(...)
		watcher.calls[#watcher.calls + 1] = {...}
	end

	return watcher
end
function CallWatcher.patch(xs, key)
	assert(type(xs) == "table", "xs must be a table")

	local watcher = CallWatcher.create(xs[key])
	xs[key] = watcher.call
	return watcher
end

local Test = {}
Test.__index = Test
function Test:run()
	logging.debug("Running test %s.%s", self.suite_name, self.name)

	local ok = debugging.protected_call(self.fn)
	if not ok then
		logging.info("Failed test %s.%s", self.suite_name, self.name)
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
	assert(not pcall(fn, ...), "fn must raise an error")
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
end
function testing.run_all()
	logging.info("Running all tests")

	local ok = true
	for _, test_suite in ipairs(testing.test_suites) do
		if not test_suite:run() then
			ok = false
		end
	end

	if not ok then
		logging.info("Not all tests completed successfully")
		return false
	end

	logging.debug("Completed all tests")
	return true
end

return testing
