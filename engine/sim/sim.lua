local debugging = require("engine/core/debugging")
local logging = require("engine/core/logging")
local testing = require("engine/core/testing")

local Status = {
	init = "init",
	run = "run",
	stop = "stop",
	error = "error",
}

local Sim = {}
Sim.__index = Sim
Sim.Status = Status

function Sim:broadcast(event, ...)
	assert(self._is_sim_instance)
	assert(self._status == Status.run)
	assert(type(event) == "string")

	for _, sys in ipairs(self._systems) do
		local event_handler = sys[event]
		if event_handler then
			event_handler(sys, ...)
		end
	end
end
function Sim:protected_broadcast(event, ...)
	assert(self._is_sim_instance)
	assert(self._status == Status.run)
	assert(type(event) == "string")

	for _, sys in ipairs(self._systems) do
		local event_handler = sys[event]
		if event_handler then
			debugging.protected_call(event_handler, sys, ...)
		end
	end
end
function Sim:init_sys(sys_metatable)
	assert(self._is_sim_instance)
	assert(self._status == Status.init)
	assert(type(sys_metatable) == "table")
	assert(sys_metatable._is_sys_instance == nil)
	assert(sys_metatable.__index ~= nil)

	for _, sys in ipairs(self._systems) do
		if getmetatable(sys) == sys_metatable then
			if not sys._is_sim_initialized then
				logging.warn("sys added but not initialized, likely a circular dependency")
			end
			return sys
		end
	end

	local sys = {_is_sys_instance = true, _is_sim_initialized = false}
	setmetatable(sys, sys_metatable)

	self._systems[#self._systems + 1] = sys

	if sys.on_init ~= nil then
		sys:on_init(self)
	end

	sys._is_sim_initialized = true

	return sys
end
function Sim:set_status(new_status)
	assert(Status[new_status] ~= nil)
	assert(self._is_sim_instance)

	if (self._status == Status.init) and (new_status == Status.run) then
		self._status = Status.run
	elseif (self._status == Status.run) and (new_status == Status.stop) then
		self._status = Status.stop
	elseif (new_status == Status.error) then
		self._status = Status.error
	else
		logging.error("unsupported transition from %s to %s", self._status, new_status)
		self._status = Status.error
	end
end
function Sim.create(rules, state)
	local sim = {
		_is_sim_instance = true,
		_status = Sim.Status.init,
		_systems = {},
		rules = rules,
		state = state,
	}
	setmetatable(sim, Sim)
	return sim
end
function Sim:step()
	assert(self._is_sim_instance)
	assert(self._status == Status.run)
	self:protected_broadcast("on_step")

	-- TODO extra debug-only validation:
	-- TODO ensure state and rules are serializable
	-- TODO backup rules on set_status(run), ensure rules are unchanged
end

testing.add_suite("sim.sim", {
	run = function()
		local TestSys = {}
		TestSys.__index = TestSys
		local init_patch = testing.CallWatcher.patch(TestSys, "on_init")
		local step_patch = testing.CallWatcher.patch(TestSys, "on_step")

		local test_sim = Sim.create()
		test_sim:init_sys(TestSys)
		test_sim:set_status(Status.run)
		test_sim:step()
		test_sim:set_status(Status.stop)

		init_patch:assert_called_once()
		step_patch:assert_called_once()
	end,
	add_sys_infinite_loop_fails = function()
		local TestSys = {}
		TestSys.__index = TestSys
		TestSys.on_init = function(sim)
			sim:init_sys(TestSys)
		end

		local test_sim = Sim.create()
		testing.assert_fails(function()
			test_sim:init_sys(TestSys)
		end)
	end,
})
testing.run_all()

return Sim
