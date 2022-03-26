local Debugging = require("engine/core/debugging")
local Logging = require("engine/core/logging")
local Testing = require("engine/core/testing")
local Shim = require("engine/core/shim")
local Container = require("engine/core/container")
local Schema = require("engine/core/Schema")

local debug_checks_enabled = Debugging.debug_checks_enabled

local Sim = {}

Sim.Status = {
	new = "new",
	started = "started",
	finalized = "finalized",
}

Sim.Sys = {}
Sim.Sys.__index = Sim.Sys
Sim.Sys.Schema = Schema.PartialObject{
	sys_name = Schema.String,
	sim = Schema.PartialObject{_is_sim_instance = Schema.Optional(Schema.Const(true))},
	state = Schema.SerializableObject,
	settings = Schema.SerializableObject,
	draw = Schema.Optional(Schema.Function),
	on_init = Schema.Optional(Schema.Function),
	on_start = Schema.Optional(Schema.Function),
	on_step = Schema.Optional(Schema.Function),
	on_stop = Schema.Optional(Schema.Function),
	_is_sys = Schema.Const(true),
	_is_sys_instance = Schema.Const(true),
}
Sim.Sys.metatable_schema = Schema.PartialObject{
	_is_sys = Schema.Const(true),
	_is_sys_instance = Schema.Optional(Schema.Const(false)),
}
Sim.Sys._is_sys = true
function Sim.Sys.new_metatable(sys_name, metatable)
	assert(Schema.LabelString(sys_name))
	assert(Schema.Optional(Sim.Sys.metatable_schema)(metatable))
	metatable = metatable or Sim.Sys

	local sys_metatable = {
		sys_name = sys_name,
		_is_sys_instance = false,
	}
	sys_metatable.__index = sys_metatable
	setmetatable(sys_metatable, metatable)
	assert(metatable.metatable_schema(sys_metatable))

	return sys_metatable
end

Sim.Sim = {}
Sim.Sim.Schema = Schema.PartialObject{
	state = Schema.SerializableObject,
	settings = Schema.SerializableObject,
	status = Schema.Enum(Shim.unpack(Container.get_keys(Sim.Status))),
	step_count = Schema.NonNegativeInteger,
	finalize_enqueued = Schema.Boolean,
	_is_sim = Schema.Const(true),
	_is_sim_instance = Schema.Const(true),
	_systems = Schema.Mapping(Schema.String, Sim.Sys.Schema),
	_systems_by_init_order = Schema.Array(Sim.Sys.Schema),
	_systems_by_event_cached = Schema.Mapping(Schema.String, Schema.Array(Sim.Sys.Schema)),
}
Sim.Sim.metatable_schema = Schema.PartialObject{
	_is_sim = Schema.Const(true),
	_is_sim_instance = Schema.Optional(Schema.Const(false)),
}
Sim.Sim.__index = Sim.Sim
Sim.Sim._is_sim = true
Sim.Sim.Sys = Sim.Sys
function Sim.Sim.new(state, settings, metatable)
	assert(Schema.Optional(Schema.SerializableObject)(state))
	assert(Schema.Optional(Schema.SerializableObject)(settings))
	assert(Schema.Optional(Sim.Sim.metatable_schema)(metatable))
	state = state or {}
	settings = settings or {}
	metatable = metatable or Sim.Sim

	local sim_instance = {
		state = state,
		settings = settings,
		status = Sim.Status.new,
		step_count = 0,
		finalize_enqueued = false,
		_is_sim_instance = true,
		_systems = {},
		_systems_by_init_order = {},
		_systems_by_event_cached = {},
	}
	setmetatable(sim_instance, metatable)

	if debug_checks_enabled then
		assert(metatable.Schema(sim_instance))
	end

	return sim_instance
end
function Sim.Sim:require(sys_metatable)
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(self.Sys.metatable_schema(sys_metatable))
		assert(self.status == Sim.Status.new)
	end

	local sys_name = sys_metatable.sys_name
	local sys = self._systems[sys_name]
	if sys ~= nil then
		Logging.trace("returning existing system %s", sys_name)
		return sys
	end

	Logging.debug("building system %s", sys_name)
	sys = {
		sys_name = sys_name,
		sim = self,
		state = self.state[sys_name] or {},
		settings = self.settings[sys_name] or {},
		_is_sys_instance = true,
	}
	setmetatable(sys, sys_metatable)

	-- register name _before_ init, to tolerate circular dependencies
	self._systems[sys_name] = sys

	if sys.on_init ~= nil then
		sys:on_init(self)
	end

	-- register order _after_ init, so dependencies receive messages first
	self._systems_by_init_order[#self._systems_by_init_order + 1] = sys

	-- clear event cache
	self._systems_by_event_cached = {}

	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(self.Sys.Schema(sys))
	end

	Logging.debug("built system %s", sys_name)

	return sys
end
function Sim.Sim:get(sys_metatable)
	local sys_name = sys_metatable.sys_name
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(self.Sys.metatable_schema(sys_metatable))
		assert(Schema.LabelString(sys_name))
	end

	return self._systems[sys_name]
end
function Sim.Sim:get_all()
	return self._systems_by_init_order
end
function Sim.Sim:broadcast(event_name, ...)
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(Schema.LabelString(event_name))
		assert(Schema.SerializableArray({...}))
		assert(self.status == Sim.Status.started)
	end

	local event_systems = self._systems_by_event_cached[event_name]
	if event_systems == nil then
		event_systems = self:_cache_systems_for_event(event_name)
	end

	for _, sys in ipairs(event_systems) do
		sys[event_name](sys, ...)
	end

	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
	end
end
function Sim.Sim:broadcast_pcall(event_name, ...)
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(Schema.LabelString(event_name))
		assert(Schema.SerializableArray({...}))
		assert(self.status == Sim.Status.started)
	end

	local event_systems = self._systems_by_event_cached[event_name]
	if event_systems == nil then
		event_systems = self:_cache_systems_for_event(event_name)
	end

	local send_ok = true
	for _, sys in ipairs(event_systems) do
		local result, err = Debugging.pcall(sys[event_name], sys, ...)
		if result == false then
			Logging.error("broadcast(%s) failed for sys_name=%s, err=%s", event_name, sys.sys_name, err)
			send_ok = false
		end
	end

	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
	end
	return send_ok
end
function Sim.Sim:start()
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(self.status == Sim.Status.new)
	end

	self.status = Sim.Status.started
	self:broadcast("on_start")

	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
	end
end
function Sim.Sim:step()
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(self.status == Sim.Status.started)
	end

	self:broadcast("on_step")

	self.step_count = self.step_count + 1

	if self.finalize_enqueued then
		self:finalize()
	end

	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
	end
end
function Sim.Sim:enqueue_finalize()
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(self.status == Sim.Status.started)
	end

	self.finalize_enqueued = true
end
function Sim.Sim:finalize()
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(self.status ~= Sim.Status.finalized)
	end

	if self.status == Sim.Status.started then
		self:broadcast("on_finalize")
	end

	self.status = Sim.Status.finalized

	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
	end
end
function Sim.Sim:run()
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(self.status == Sim.Status.new)
	end

	self:start()
	while self.status == Sim.Status.started do
		self:step()
	end
end
function Sim.Sim:_cache_systems_for_event(event_name)
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(Schema.LabelString(event_name))
	end

	Logging.debug("regenerating cache of systems for event_name %s", event_name)

	local event_systems = {}
	for _, sys in ipairs(self._systems_by_init_order) do
		if sys[event_name] ~= nil then
			event_systems[#event_systems + 1] = sys
		end
	end
	self._systems_by_event_cached[event_name] = event_systems

	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
	end

	return event_systems
end

Sim.tests = Testing.add_suite("engine.sim", {
	require = function()
		local TestSys = Sim.Sys.new_metatable("test")
		local init_patch = Testing.CallWatcher.patch(TestSys, "on_init")

		local sim_instance = Sim.Sim.new()
		sim_instance:require(TestSys)

		init_patch:assert_called_once()
	end,
	require_circular_dependency = function()
		local TestSys = Sim.Sys.new_metatable("test")
		function TestSys:on_init(sim_instance)
			sim_instance:require(TestSys)
			self.init_reached = true
		end

		local sim_instance = Sim.Sim.new()
		local sys = sim_instance:require(TestSys)
		assert(sys.init_reached)
	end,
	get = function()
		local TestSys = Sim.Sys.new_metatable("test")

		local sim_instance = Sim.Sim.new()
		assert(sim_instance:get(TestSys) == nil)

		local sys = sim_instance:require(TestSys)
		assert(sim_instance:get(TestSys) == sys)
	end,
	broadcast = function()
		local TestSys = Sim.Sys.new_metatable("test")
		local test_event_patch = Testing.CallWatcher.patch(TestSys, "on_test_event")

		local sim_instance = Sim.Sim.new()
		sim_instance:require(TestSys)
		sim_instance:start()

		test_event_patch:assert_not_called()
		local args = {1, '2', 3.45}
		sim_instance:broadcast("on_test_event", Shim.unpack(args))

		test_event_patch:assert_called_once()
		Container.assert_equal(Container.array_slice(test_event_patch.calls[1], 2), args)
	end,
	send_error_raises = function()
		local TestSys = Sim.Sys.new_metatable("test")
		TestSys.on_test_event = function() error() end

		local sim_instance = Sim.Sim.new()
		sim_instance:require(TestSys)
		sim_instance:start()

		Testing.assert_fails(function()
			sim_instance:broadcast("on_test_event")
		end)
	end,
	broadcast_pcall = function()
		local TestSys = Sim.Sys.new_metatable("test")
		TestSys.on_test_event = function() error() end

		local sim_instance = Sim.Sim.new()
		sim_instance:require(TestSys)
		sim_instance:start()

		Testing.suppress_errors(function()
			assert(sim_instance:broadcast_pcall("on_test_event") == false)
		end)
	end,
	step = function()
		local TestSys = Sim.Sys.new_metatable("test")

		local sim_instance = Sim.Sim.new()
		sim_instance:require(TestSys)
		sim_instance:start()
		local step_count = 10
		for _ = 1, step_count do
			sim_instance:step()
		end
		assert(sim_instance.step_count == step_count)

		sim_instance:enqueue_finalize()
		sim_instance:step()
		assert(sim_instance.status == Sim.Status.finalized)
	end,
})

return Sim
