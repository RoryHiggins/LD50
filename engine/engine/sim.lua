local debugging = require("engine/core/debugging")
local logging = require("engine/core/logging")
local testing = require("engine/core/testing")
local serialization = require("engine/core/serialization")
local shim = require("engine/core/shim")
local container = require("engine/core/container")

local Status = {
	new = "new",
	running = "running",
	stopped = "stopped",
}

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
Sim.Status = Status
function Sim.new(state, settings)
	if state == nil then
		state = {}
	end
	assert(type(state) == "table")

	if settings == nil then
		settings = {}
	end
	assert(type(settings) == "table")

	local sim = {
		state = state,
		settings = settings,
		status = Status.new,
		step_count = 0,
		_is_sim_instance = true,
		_systems = {},
		_systems_ordered = {},
		_systems_by_event_cached = {},
	}
	return setmetatable(sim, Sim)
end
function Sim:require(sys_metatable)
	assert(self.status == Status.new)
	assert(type(sys_metatable) == "table" and sys_metatable._is_sys and not sys_metatable._is_sys_instance,
		   "sys_metatable must be a table returned by Sys.new_metatable()")

	local sys_name = sys_metatable.sys_name
	assert(type(sys_name) == "string", "system name must be a string")

	local sys = self._systems[sys_name]
	if sys ~= nil then
		logging.trace("returning existing system %s", sys_name)
		return sys
	end

	logging.debug("building system %s", sys_name)
	sys = {
		_is_sys_instance = true,
		_is_sys_init_complete = false,
	}
	setmetatable(sys, sys_metatable)

	self._systems[sys_name] = sys
	self._systems_ordered[#self._systems_ordered + 1] = sys
	self._systems_by_event_cached = {}  -- clear event cache

	if sys.on_require ~= nil then
		sys:on_require(self)
	end

	sys._is_sys_init_complete = true

	logging.debug("built system %s", sys_name)

	return sys
end
function Sim:get(sys_name)
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
function Sim:broadcast(event_name, ...)
	if debugging.debug_checks_enabled then
		assert(self._is_sim_instance)
		assert(type(event_name) == "string")
		assert(self.status == Status.running)
	end

	local event_systems = self._systems_by_event_cached[event_name]
	if event_systems == nil then
		event_systems = self:_cache_systems_for_event(event_name)
	end

	for _, sys in ipairs(event_systems) do
		sys[event_name](sys, ...)
	end
end
function Sim:broadcast_pcall(event_name, ...)
	if debugging.debug_checks_enabled then
		assert(self._is_sim_instance)
		assert(type(event_name) == "string")
		assert(self.status == Status.running)
	end

	local event_systems = self._systems_by_event_cached[event_name]
	if event_systems == nil then
		event_systems = self:_cache_systems_for_event(event_name)
	end

	local send_ok = true
	for _, sys in ipairs(event_systems) do
		local result, err = debugging.pcall(sys[event_name], sys, ...)
		if result == false then
			logging.error("broadcast(%s) failed for sys_name=%s, err=%s", event_name, sys.sys_name, err)
			send_ok = false
		end
	end
	return send_ok
end
function Sim:step()
	if debugging.debug_checks_enabled then
		assert(self._is_sim_instance)
		assert(self.status == Status.running)
	end

	self:broadcast("on_step")

	self.step_count = self.step_count + 1

	if debugging.debug_checks_enabled then
		serialization.check_serializable(self.state)
	end
end
function Sim:set_status(new_status)
	assert(self._is_sim_instance)
	assert(type(new_status) == "string")
	assert(Status[new_status] ~= nil)

	if (self.status == Status.new) and (new_status == Status.running) then
		self.status = Status.running
		self:broadcast("on_start")
	elseif (self.status == Status.running) and (new_status == Status.stopped) then
		self:broadcast("on_stop")
		self.status = Status.stopped
	else
		logging.error("unsupported state transition from %s to %s", self.status, new_status)
	end

	assert(type(self.status) == "string")
	assert(Status[self.status] ~= nil)
end
function Sim:get_state(sys)
	assert(self._is_sim_instance)
	assert(self.status == Status.new)
	assert(sys._is_sys_instance)

	local state = self.state[sys.sys_name]
	if state == nil then
		state = {}
		self.state[sys.sys_name] = state
	end
	assert(type(state) == "table")

	return state
end
function Sim:get_settings(sys)
	assert(self._is_sim_instance)
	assert(self.status == Status.new)
	assert(sys._is_sys_instance)

	local settings = self.settings[sys.sys_name]
	if settings == nil then
		settings = {}
		self.settings[sys.sys_name] = settings
	end
	assert(type(settings) == "table")

	return settings
end

Sim.tests = testing.add_suite("engine.sim", {
	require = function()
		local TestSys = Sys.new_metatable("test")
		local init_patch = testing.CallWatcher.patch(TestSys, "on_require")

		local sim = Sim.new()
		sim:require(TestSys)

		init_patch:assert_called_once()
	end,
	require_circular_dependency = function()
		local TestSys = Sys.new_metatable("test")
		function TestSys:on_require(sim)
			sim:require(TestSys)
			self.init_reached = true
		end

		local sim = Sim.new()
		local sys = sim:require(TestSys)
		assert(sys.init_reached)
	end,
	get = function()
		local TestSys = Sys.new_metatable("test")

		local sim = Sim.new()
		assert(sim:get("test") == nil)

		local sys = sim:require(TestSys)
		assert(sim:get("test") == sys)
	end,
	broadcast = function()
		local TestSys = Sys.new_metatable("test")
		local test_event_patch = testing.CallWatcher.patch(TestSys, "on_test_event")

		local sim = Sim.new()
		sim:require(TestSys)
		sim:set_status(Status.running)

		test_event_patch:assert_not_called()
		local args = {1, '2', 3.45}
		sim:broadcast("on_test_event", shim.unpack(args))

		test_event_patch:assert_called_once()
		container.assert_equal(container.array_slice(test_event_patch.calls[1], 2), args)
	end,
	send_error_raises = function()
		local TestSys = Sys.new_metatable("test")
		TestSys.on_test_event = function() error() end

		local sim = Sim.new()
		sim:require(TestSys)
		sim:set_status(Status.running)

		testing.assert_fails(function()
			sim:broadcast("on_test_event")
		end)
	end,
	broadcast_pcall = function()
		local TestSys = Sys.new_metatable("test")
		TestSys.on_test_event = function() error() end

		local sim = Sim.new()
		sim:require(TestSys)
		sim:set_status(Status.running)

		testing.suppress_log_errors(function()
			assert(sim:broadcast_pcall("on_test_event") == false)
		end)
	end,
	step = function()
		local TestSys = Sys.new_metatable("test")

		local sim = Sim.new()
		sim:require(TestSys)
		sim:set_status(Status.running)
		local step_count = 10
		for _ = 1, step_count do
			sim:step()
		end
		assert(sim.step_count == step_count)
	end,
})

return Sim
