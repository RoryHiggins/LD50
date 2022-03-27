local Debugging = require("engine/core/debugging")
local Logging = require("engine/core/logging")
local Testing = require("engine/core/testing")
local Shim = require("engine/core/shim")
local Schema = require("engine/core/Schema")
local Container = require("engine/core/container")
local Model = require("engine/core/model")

local debug_checks_enabled = Debugging.debug_checks_enabled

local Sim = {}

Sim.Status = Model.Enum("new","started", "finalized")

Sim.Sys = {}
Sim.Sys.__index = Sim.Sys
Sim.Sys.Schema = Schema.PartialObject{
	sys_name = Schema.String,
	sim = Schema.PartialObject{_is_sim_instance = Schema.Optional(Schema.Const(true))},
	state = Schema.SerializableObject,
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
	status = Sim.Status.Schema,
	step_id = Schema.PositiveInteger,
	stopping = Schema.Boolean,
	_initial_state = Schema.SerializableObject,
	_message_history = Schema.Array(Schema.SerializableArray),
	_is_sim = Schema.Const(true),
	_is_sim_instance = Schema.Const(true),
	_systems = Schema.Mapping(Schema.String, Sim.Sys.Schema),
	_event_listeners_ordered = Schema.Array(Schema.AnyObject),
	_event_listeners_cached = Schema.Mapping(Schema.String, Schema.Array(Schema.AnyObject)),
}
Sim.Sim.metatable_schema = Schema.PartialObject{
	_is_sim = Schema.Const(true),
	_is_sim_instance = Schema.Optional(Schema.Const(false)),
}
Sim.Sim.__index = Sim.Sim
Sim.Sim._is_sim = true
Sim.Sim.Sys = Sim.Sys
function Sim.Sim.new(state, metatable)
	assert(Schema.Optional(Schema.SerializableObject)(state))
	assert(Schema.Optional(Sim.Sim.metatable_schema)(metatable))
	state = state or {}
	metatable = metatable or Sim.Sim

	local sim_instance = {
		state = state,
		status = Sim.Status.new,
		step_id = 1,
		stopping = false,
		_initial_state = state,
		-- TODO write to _message_history
		_message_history = {},
		_is_sim_instance = true,
		_systems = {},
		_event_listeners_ordered = {},
		_event_listeners_cached = {},
	}
	setmetatable(sim_instance, metatable)
	sim_instance._event_listeners_ordered[1] = sim_instance

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
		_is_sys_instance = true,
	}
	setmetatable(sys, sys_metatable)

	-- register name _before_ init, to tolerate circular dependencies
	self._systems[sys_name] = sys

	if sys.on_init ~= nil then
		sys:on_init(self)
	end

	-- register order _after_ init, so dependencies receive messages first
	self._event_listeners_ordered[#self._event_listeners_ordered + 1] = sys

	-- clear event cache
	self._event_listeners_cached = {}

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
	return self._event_listeners_ordered
end
function Sim.Sim:broadcast(event_name, ...)
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(Schema.LabelString(event_name))
		assert(Schema.SerializableArray({...}))
		assert(self.status == Sim.Status.started)
	end

	local event_systems = self._event_listeners_cached[event_name]
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

	local event_systems = self._event_listeners_cached[event_name]
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
		assert(self.status ~= Sim.Status.finalized)
	end

	local old_status = self.status

	self.status = Sim.Status.started

	if old_status == Sim.Status.new then
		self:broadcast("on_start")
	end

	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
	end
end
function Sim.Sim:step()
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(self.status == Sim.Status.started)
		assert(not self.stopping)
	end

	self:broadcast_pcall("on_step")

	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
	end
end
function Sim.Sim:stop()
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(self.status ~= Sim.Status.finalized)
	end

	self.stopping = true
end
function Sim.Sim:finalize()
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
	end

	if self.status == Sim.Status.started then
		self:broadcast_pcall("on_finalize")
	end

	self.status = Sim.Status.finalized

	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
	end
end
function Sim.Sim:running()
	return self.status == Sim.Status.started and not self.stopping
end
function Sim.Sim:run()
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(self.status == Sim.Status.new)
	end

	self:start()
	while self:running() do
		self:step()
	end
	self:finalize()
end
function Sim.Sim:on_step()
	self.step_id = self.step_id + 1
end
function Sim.Sim:_cache_systems_for_event(event_name)
	if debug_checks_enabled then
		assert(Sim.Sim.Schema(self))
		assert(Schema.LabelString(event_name))
	end

	Logging.debug("regenerating cache of systems for event_name %s", event_name)

	local event_systems = {}

	for _, sys in ipairs(self._event_listeners_ordered) do
		if sys[event_name] ~= nil then
			event_systems[#event_systems + 1] = sys
		end
	end
	self._event_listeners_cached[event_name] = event_systems

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

		Testing.assert_fails(function()
			sim_instance:step()
		end)

		sim_instance:require(TestSys)
		sim_instance:start()
		sim_instance:start()  -- ensure idempotent
		for step_id = 1, 10 do
			assert(sim_instance.step_id == step_id)
			sim_instance:step()
		end

		sim_instance:stop()
		sim_instance:stop()  -- ensure idempotent

		Testing.assert_fails(function()
			sim_instance:step()
		end)

		assert(sim_instance:running() == false)
		sim_instance:finalize()
		sim_instance:finalize()  -- ensure idempotent

		Testing.assert_fails(function()
			sim_instance:step()
		end)

		Testing.assert_fails(function()
			sim_instance:start()
		end)

		sim_instance = Sim.Sim.new()
		sim_instance:stop()
		sim_instance:finalize()
		sim_instance:finalize()  -- ensure idempotent
		Testing.assert_fails(function()
			sim_instance:stop()
		end)
	end,
})

return Sim
