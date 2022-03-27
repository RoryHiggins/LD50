local Shim = require("engine/core/shim")
local Debugging = require("engine/core/debugging")
local Testing = require("engine/core/testing")
local Logging = require("engine/core/logging")
local Schema = require("engine/core/Schema")
local Container = require("engine/core/container")
local Model = require("engine/core/model")
local Client = require("engine/engine/client")
local World = require("engine/engine/world")

local debug_checks_enabled = Debugging.debug_checks_enabled

local Controller = {}
Controller.InputName = Model.Enum(
	"up", "held", "left", "right", "a", "b", "x", "y", "start"
)
Controller.Input = {}
Controller.Input.Schema = Schema.Object{
	held = Schema.Boolean,
	step_id = Schema.NonNegativeInteger,
}
Controller.Input.defaults = {
	held = false,
	step_id = 0,
}
Controller.Controller = {}
Controller.Controller.Schema = Schema.Object{
	inputs =  Schema.Mapping(Controller.InputName.Schema, Controller.Input.Schema),
}
Controller.Controller.defaults = {
	inputs = {},
}
for _, input_name in ipairs(Controller.InputName.enum_values) do
	Controller.Controller.defaults.inputs[input_name] = Controller.Input.defaults
end
Controller.Controller.max_id = 4

Controller.KeyboardKeys = Model.AnyEnum(Shim.unpack(Client.wrappers.Window.get_key_names()))
Controller.BindingType = Model.Enum("keyboard")
Controller.Binding = {}
Controller.Binding.Schema = Schema.Object{
	controller_id = Schema.BoundedInteger(1, Controller.Controller.max_id),
	input_name = Controller.InputName.Schema,
	binding_type = Controller.BindingType.Schema,
	binding_keyboard_key = Schema.Optional(Controller.KeyboardKeys.Schema),
}

Controller.WorldSys = World.Sys.new_metatable("controller")
Controller.WorldSys.default_id = 1
Controller.WorldSys.State = {}
Controller.WorldSys.State.Schema = Schema.Object{
	controllers = Schema.Array(Controller.Controller.Schema, Controller.Controller.max_id),
}
Controller.WorldSys.State.defaults = {
	controllers = {}
}
for i = 1, Controller.Controller.max_id do
	Controller.WorldSys.State.defaults.controllers[i] = Controller.Controller.defaults
end
function Controller.WorldSys:find(controller_id)
	if debug_checks_enabled then
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
	end

	return self.state.controllers[controller_id]
end
function Controller.WorldSys:get_default()
	return self:find(self.default_id)
end
function Controller.WorldSys:get_held(controller_id, input_name)
	if debug_checks_enabled then
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
		assert(Controller.InputName.Schema(input_name))
	end

	return self.state.controllers[controller_id].inputs[input_name].held
end
function Controller.WorldSys:get_toggled(controller_id, input_name)
	if debug_checks_enabled then
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
		assert(Controller.InputName.Schema(input_name))
	end

	return self.sim.step_id == self.state.controllers[controller_id].inputs[input_name].step_id
end
function Controller.WorldSys:get_pressed(controller_id, input_name)
	return self:get_held(controller_id, input_name) and self:get_toggled(controller_id, input_name)
end
function Controller.WorldSys:get_released(controller_id, input_name)
	return not self:get_held(controller_id, input_name) and self:get_toggled(controller_id, input_name)
end
function Controller.WorldSys:on_init()
	Container.set_defaults(self.state, Controller.WorldSys.State.defaults)
	if debug_checks_enabled then
		assert(Controller.WorldSys.State.Schema(self.state))
	end
end
function Controller.WorldSys:on_input_set(controller_id, input_name, held)
	if debug_checks_enabled then
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
		assert(Controller.InputName.Schema(input_name))
		assert(Schema.Boolean(held))
	end

	local input = self.state.controllers[controller_id].inputs[input_name]
	input.held = held
	input.step_id = self.sim.step_id
end

Controller.tests = Testing.add_suite("engine.controller", {
	run_world = function()
		local world_sim = World.World.new()
		local controller_world = world_sim:require(Controller.WorldSys)
		world_sim:start()
		assert(Controller.Controller.Schema(controller_world:find(controller_world.default_id)))
		assert(Controller.Controller.Schema(controller_world:get_default()))
		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == false)
		world_sim:step()
		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == false)

		controller_world:on_input_set(controller_world.default_id, Controller.InputName.up, true)
		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == false)

		world_sim:step()
		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == false)

		controller_world:on_input_set(controller_world.default_id, Controller.InputName.up, false)
		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == true)

		world_sim:finalize()
	end
})

return Controller
