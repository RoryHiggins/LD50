local debugging = require("engine/core/debugging")
local logging = require("engine/core/logging")
local testing = require("engine/core/testing")
local shim = require("engine/core/shim")
local container = require("engine/core/container")

local Sys = {}
Sys.__index = Sys
Sys._is_sys = true
function Sys.new_metatable(sys_name)
	assert(type(sys_name) == "string" and string.match(sys_name, "[a-zA-Z_][a-zA-Z0-9_]*") == sys_name,
		   "system name must be an alphanumeric string (alphabet, digits, underscore), and must not start with a digit")

	local sys_metatable = {
		sys_name = sys_name
	}
	sys_metatable.__index = sys_metatable
	return setmetatable(sys_metatable, Sys)
end

local Sim = {}
Sim.__index = Sim
Sim._is_sim = true
Sim.Sys = Sys
function Sim.new(state)
	if state == nil then
		state = {}
	end
	assert(type(state) == "table")

	local sim = {
		_is_sim_instance = true,
		_state = state,
		_systems = {},
		_systems_ordered = {},
		_systems_by_event_cached = {},
	}
	return setmetatable(sim, Sim)
end
function Sim:require(sys_metatable, allow_circular_dependency)
	assert(type(sys_metatable) == "table" and sys_metatable._is_sys and not sys_metatable._is_sys_instance,
		   "sys_metatable must be a table returned by Sys.new_metatable()")

	local sys_name = sys_metatable.sys_name
	assert(type(sys_name) == "string", "system name must be a string")

	local sys = self._systems[sys_name]
	if sys ~= nil then
		assert(allow_circular_dependency == true or sys._is_sys_init_complete == true,
			   "circular dependency detected, must explicitly allow with allow_circular_dependency true")

		logging.trace("returning existing system %s", sys_name)
		return sys
	end

	logging.debug("building system %s", sys_name)
	sys = setmetatable({}, sys_metatable)
	sys._is_sys_instance = true
	sys._is_sys_init_complete = false
	self._systems[sys_name] = sys
	self._systems_ordered[#self._systems_ordered + 1] = sys
	self._systems_by_event_cached = {}  -- new type can invalidate cache, clearing

	if sys.on_init ~= nil then
		if self._state[sys_name] == nil then
			self._state[sys_name] = {}
		end
		assert(type(self._state[sys_name]) == "table")

		sys:on_init(self, self._state[sys_name])
	end

	sys._is_sys_init_complete = true
	logging.debug("built system %s", sys_name)

	return sys
end
function Sim:find_by_name(sys_name)
	assert(type(sys_name) == "string")
	assert(string.match(sys_name, "[a-zA-Z_][a-zA-Z0-9_]*") == sys_name)

	return self._systems[sys_name]
end
function Sim:_cache_systems_for_event(event_name)
	assert(type(event_name) == "string")
	assert(string.match(event_name, "on_[a-zA-Z_][a-zA-Z0-9_]*") == event_name)

	logging.debug("regenerating cache of systems for event_name %s", event_name)
	local event_systems = {}
	for _, sys in ipairs(self._systems_ordered) do
		if sys[event_name] ~= nil then
			event_systems[#event_systems + 1] = sys
		end
	end

	self._systems_by_event_cached[event_name] = event_systems
	return event_systems
end
function Sim:send(event_name, ...)
	local event_systems = self._systems_by_event_cached[event_name]
	if event_systems == nil then
		event_systems = self:_cache_systems_for_event(event_name)
	end

	for _, sys in ipairs(event_systems) do
		sys[event_name](sys, ...)
	end
end
function Sim:send_pcall(event_name, ...)
	local event_systems = self._systems_by_event_cached[event_name]
	if event_systems == nil then
		event_systems = self:_cache_systems_for_event(event_name)
	end

	local send_ok = true
	for _, sys in ipairs(event_systems) do
		local result, err = debugging.pcall(sys[event_name], sys, ...)
		if result == false then
			logging.error("send(%s) failed for sys_name=%s, err=%s", event_name, sys.sys_name, err)
			send_ok = false
		end
	end
	return send_ok
end

testing.add_suite("engine.sim", {
	require = function()
		local TestSys = Sys.new_metatable("test")
		local init_patch = testing.CallWatcher.patch(TestSys, "on_init")

		local sim = Sim.new()
		sim:require(TestSys)

		init_patch:assert_called_once()
	end,
	require_circular_dependency_fails = function()
		local TestSys = Sys.new_metatable("test")
		function TestSys:on_init(sim)
			sim:require(TestSys)
			self.init_complete = true
		end

		local sim = Sim.new()
		testing.assert_fails(function()
			sim:require(TestSys)
		end)
	end,
	require_circular_dependency_explicltly_allowed = function()
		local TestSys = Sys.new_metatable("test")
		function TestSys:on_init(sim)
			sim:require(TestSys, true)
			self.init_complete = true
		end

		local sim = Sim.new()
		local sys = sim:require(TestSys, true)
		assert(sys.init_complete)
	end,
	find_by_name = function()
		local TestSys = Sys.new_metatable("test")

		local sim = Sim.new()
		assert(sim:find_by_name("test") == nil)

		local sys = sim:require(TestSys)
		assert(sim:find_by_name("test") == sys)
	end,
	send = function()
		local TestSys = Sys.new_metatable("test")
		local test_event_patch = testing.CallWatcher.patch(TestSys, "on_test_event")

		local sim = Sim.new()
		sim:require(TestSys)

		test_event_patch:assert_not_called()
		local args = {1, '2', 3.45}
		sim:send("on_test_event", shim.unpack(args))

		test_event_patch:assert_called_once()
		container.assert_equal(container.array_slice(test_event_patch.calls[1], 2), args)
	end,
	send_error_raises = function()
		local TestSys = Sys.new_metatable("test")
		TestSys.on_test_event = function() error() end

		local sim = Sim.new()
		sim:require(TestSys)
		testing.assert_fails(function()
			sim:send("on_test_event")
		end)
	end,
	send_pcall = function()
		local TestSys = Sys.new_metatable("test")
		TestSys.on_test_event = function() error() end

		local sim = Sim.new()
		sim:require(TestSys)
		testing.suppress_log_errors(function()
			assert(sim:send_pcall("on_test_event") == false)
		end)
	end,
})

return Sim
