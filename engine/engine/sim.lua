local debugging = require("engine/core/debugging")
local logging = require("engine/core/logging")
local testing = require("engine/core/testing")
local shim = require("engine/core/shim")
local container = require("engine/core/container")
local schema = require("engine/core/schema")

local debug_checks_enabled = debugging.debug_checks_enabled

local Status = {
	new = "new",
	started = "started",
	finalized = "finalized",
}
local StatusSchema = schema.Enum(shim.unpack(container.table_get_keys(Status)))

local Sys = {}
Sys.__index = Sys
Sys.schema = schema.PartialObject{
	sys_name = schema.String,
	sim = schema.PartialObject{_is_sim_instance = schema.Optional(schema.Const(true))},
	state = schema.SerializableObject,
	settings = schema.SerializableObject,
	_is_sys = schema.Const(true),
	_is_sys_instance = schema.Const(true),
}
Sys.metatable_schema = schema.PartialObject{
	_is_sys = schema.Const(true),
	_is_sys_instance = schema.Optional(schema.Const(false)),
}
Sys._is_sys = true
function Sys.new_metatable(sys_name, metatable)
	assert(schema.LabelString(sys_name))
	assert(schema.Optional(Sys.metatable_schema)(metatable))
	metatable = metatable or Sys

	local sys_metatable = {
		sys_name = sys_name,
		_is_sys_instance = false,
	}
	sys_metatable.__index = sys_metatable
	setmetatable(sys_metatable, metatable)
	assert(metatable.metatable_schema(sys_metatable))

	return sys_metatable
end

local Sim = {}
Sim.schema = schema.PartialObject{
	state = schema.SerializableObject,
	settings = schema.SerializableObject,
	status = StatusSchema,
	step_count = schema.NonNegativeInteger,
	_is_sim = schema.Const(true),
	_is_sim_instance = schema.Const(true),
	_systems_by_name = schema.Mapping(schema.String, Sys.schema),
	_systems_by_init_order = schema.Array(Sys.schema),
	_systems_by_event_cached = schema.Mapping(schema.String, schema.Array(Sys.schema)),
}
Sim.metatable_schema = schema.PartialObject{
	_is_sim = schema.Const(true),
	_is_sim_instance = schema.Optional(schema.Const(false)),
}
Sim.__index = Sim
Sim._is_sim = true
Sim.Sys = Sys
function Sim.new(state, settings, metatable)
	assert(schema.Optional(schema.SerializableObject)(state))
	assert(schema.Optional(schema.SerializableObject)(settings))
	assert(schema.Optional(Sim.metatable_schema)(metatable))
	state = state or {}
	settings = settings or {}
	metatable = metatable or Sim

	local sim = {
		state = state,
		settings = settings,
		status = Status.new,
		step_count = 0,
		_is_sim_instance = true,
		_systems_by_name = {},
		_systems_by_init_order = {},
		_systems_by_event_cached = {},
	}
	setmetatable(sim, metatable)

	if debug_checks_enabled then
		assert(metatable.schema(sim))
	end

	return sim
end
function Sim:require(sys_metatable)
	if debug_checks_enabled then
		assert(Sim.schema(self))
		assert(self.Sys.metatable_schema(sys_metatable))
		assert(self.status == Status.new)
	end

	local sys_name = sys_metatable.sys_name
	local sys = self._systems_by_name[sys_name]
	if sys ~= nil then
		logging.trace("returning existing system %s", sys_name)
		return sys
	end

	logging.debug("building system %s", sys_name)
	sys = {
		sys_name = sys_name,
		sim = self,
		state = self.state[sys_name] or {},
		settings = self.settings[sys_name] or {},
		_is_sys_instance = true,
	}
	setmetatable(sys, sys_metatable)

	self._systems_by_name[sys_name] = sys
	self._systems_by_init_order[#self._systems_by_init_order + 1] = sys
	self._systems_by_event_cached = {}  -- clear event cache

	if sys.on_init ~= nil then
		sys:on_init(self)
	end

	if debug_checks_enabled then
		assert(Sim.schema(self))
		assert(self.Sys.schema(sys))
	end

	logging.debug("built system %s", sys_name)

	return sys
end
function Sim:get(sys_metatable)
	local sys_name = sys_metatable.sys_name
	if debug_checks_enabled then
		assert(Sim.schema(self))
		assert(self.Sys.metatable_schema(sys_metatable))
		assert(schema.LabelString(sys_name))
	end

	return self._systems_by_name[sys_name]
end
function Sim:_cache_systems_for_event(event_name)
	if debug_checks_enabled then
		assert(Sim.schema(self))
		assert(schema.LabelString(event_name))
	end

	logging.debug("regenerating cache of systems for event_name %s", event_name)

	local event_systems = {}
	for _, sys in ipairs(self._systems_by_init_order) do
		if sys[event_name] ~= nil then
			event_systems[#event_systems + 1] = sys
		end
	end
	self._systems_by_event_cached[event_name] = event_systems

	if debug_checks_enabled then
		assert(Sim.schema(self))
	end

	return event_systems
end
function Sim:broadcast(event_name, ...)
	if debug_checks_enabled then
		assert(Sim.schema(self))
		assert(schema.LabelString(event_name))
		assert(schema.SerializableArray({...}))
		assert(self.status == Status.started)
	end

	local event_systems = self._systems_by_event_cached[event_name]
	if event_systems == nil then
		event_systems = self:_cache_systems_for_event(event_name)
	end

	for _, sys in ipairs(event_systems) do
		sys[event_name](sys, ...)
	end

	if debug_checks_enabled then
		assert(Sim.schema(self))
	end
end
function Sim:broadcast_pcall(event_name, ...)
	if debug_checks_enabled then
		assert(Sim.schema(self))
		assert(schema.LabelString(event_name))
		assert(schema.SerializableArray({...}))
		assert(self.status == Status.started)
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

	if debug_checks_enabled then
		assert(Sim.schema(self))
	end
	return send_ok
end
function Sim:step()
	if debug_checks_enabled then
		assert(Sim.schema(self))
		assert(self.status == Status.started)
	end

	self:broadcast("on_step")

	self.step_count = self.step_count + 1

	if debug_checks_enabled then
		assert(Sim.schema(self))
	end
end
function Sim:start()
	if debug_checks_enabled then
		assert(Sim.schema(self))
		assert(self.status == Status.new)
	end

	self.status = Status.started
	self:broadcast("on_start")

	if debug_checks_enabled then
		assert(Sim.schema(self))
	end
end
function Sim:finalize()
	if debug_checks_enabled then
		assert(Sim.schema(self))
		assert(self.status ~= Status.finalized)
	end

	if self.status == Status.started then
		self:broadcast("on_finalize")
	end

	self.status = Status.finalized

	if debug_checks_enabled then
		assert(Sim.schema(self))
	end
end
function Sim:run()
	if debug_checks_enabled then
		assert(Sim.schema(self))
		assert(self.status == Status.new)
	end

	self:start()
	while self.status == Status.started do
		self:step()
	end
end

local tests = testing.add_suite("engine.sim", {
	require = function()
		local TestSys = Sys.new_metatable("test")
		local init_patch = testing.CallWatcher.patch(TestSys, "on_init")

		local sim = Sim.new()
		sim:require(TestSys)

		init_patch:assert_called_once()
	end,
	require_circular_dependency = function()
		local TestSys = Sys.new_metatable("test")
		function TestSys:on_init(sim)
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
		assert(sim:get(TestSys) == nil)

		local sys = sim:require(TestSys)
		assert(sim:get(TestSys) == sys)
	end,
	broadcast = function()
		local TestSys = Sys.new_metatable("test")
		local test_event_patch = testing.CallWatcher.patch(TestSys, "on_test_event")

		local sim = Sim.new()
		sim:require(TestSys)
		sim:start()

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
		sim:start()

		testing.assert_fails(function()
			sim:broadcast("on_test_event")
		end)
	end,
	broadcast_pcall = function()
		local TestSys = Sys.new_metatable("test")
		TestSys.on_test_event = function() error() end

		local sim = Sim.new()
		sim:require(TestSys)
		sim:start()

		testing.suppress_errors(function()
			assert(sim:broadcast_pcall("on_test_event") == false)
		end)
	end,
	step = function()
		local TestSys = Sys.new_metatable("test")

		local sim = Sim.new()
		sim:require(TestSys)
		sim:start()
		local step_count = 10
		for _ = 1, step_count do
			sim:step()
		end
		assert(sim.step_count == step_count)
	end,
})

local sim = {}
sim.model = {}
sim.model.Status = Status
sim.model.StatusSchema = StatusSchema
sim.Sys = Sys
sim.Sim = Sim
sim.tests = tests

return sim
