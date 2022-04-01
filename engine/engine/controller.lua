local Shim = require("engine/core/shim")
local Debugging = require("engine/core/debugging")
local Testing = require("engine/core/testing")
local Schema = require("engine/core/schema")
local Container = require("engine/core/container")
local Model = require("engine/core/model")
local Client = require("engine/engine/client")
local World = require("engine/engine/world")
local Game = require("engine/engine/game")

local debug_checks_enabled = Debugging.debug_checks_enabled

local Controller = {}
Controller.InputName = Model.Enum(
	"up", "down", "left", "right", "a", "b"
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
Controller.Controller.default_id = 1
Controller.Controller.max_id = 4

Controller.KeyboardKey = Model.AnyEnum(Shim.unpack(Client.Wrappers.Window.get_key_names()))

Controller.BindingType = Model.Enum("virtual", "keyboard")

Controller.Binding = {}
Controller.Binding.Schema = Schema.Object{
	type = Controller.BindingType.Schema,
	keyboard_key = Schema.Optional(Controller.KeyboardKey.Schema),
	virtual_value = Schema.Optional(Schema.Boolean),
}

Controller.InputBindings = {}
Controller.InputBindings.Schema = Schema.Object{
	bindings = Schema.Array(Controller.Binding.Schema),
}
Controller.InputBindings.defaults = {
	bindings = {}
}

Controller.ControllerBindings = {}
Controller.ControllerBindings.Schema = Schema.Object{
	inputs = Schema.Mapping(Controller.InputName.Schema, Controller.InputBindings.Schema),
}
Controller.ControllerBindings.defaults = {
	inputs = {},
}
for _, input_name in ipairs(Controller.InputName.enum_values) do
	Controller.ControllerBindings.defaults.inputs[input_name] = Controller.InputBindings.defaults
end
Controller.ControllerBindings.defaults_default_controller = Container.deep_copy(Controller.ControllerBindings.defaults)
Controller.ControllerBindings.defaults_default_controller.inputs[Controller.InputName.up] = {bindings = {
	{type = "keyboard", keyboard_key = "up"},
	{type = "keyboard", keyboard_key = "w"},
}}
Controller.ControllerBindings.defaults_default_controller.inputs[Controller.InputName.left] = {bindings = {
	{type = "keyboard", keyboard_key = "left"},
	{type = "keyboard", keyboard_key = "a"},
}}
Controller.ControllerBindings.defaults_default_controller.inputs[Controller.InputName.down] = {bindings = {
	{type = "keyboard", keyboard_key = "down"},
	{type = "keyboard", keyboard_key = "s"},
}}
Controller.ControllerBindings.defaults_default_controller.inputs[Controller.InputName.right] = {bindings = {
	{type = "keyboard", keyboard_key = "right"},
	{type = "keyboard", keyboard_key = "d"},
}}
Controller.ControllerBindings.defaults_default_controller.inputs[Controller.InputName.a] = {bindings = {
	{type = "keyboard", keyboard_key = "z"},
	{type = "keyboard", keyboard_key = "return"},
}}
Controller.ControllerBindings.defaults_default_controller.inputs[Controller.InputName.b] = {bindings = {
	{type = "keyboard", keyboard_key = "x"},
	{type = "keyboard", keyboard_key = "backspace"},
}}

Controller.WorldSys = World.Sys.new_metatable("controller")
Controller.WorldSys.default_id = Controller.Controller.default_id
Controller.WorldSys.State = {}
Controller.WorldSys.State.Schema = Schema.Object{
	controllers = Schema.BoundedArray(Controller.Controller.Schema, 1, Controller.Controller.max_id),
}
Controller.WorldSys.State.defaults = {
	controllers = {}
}
for i = 1, Controller.Controller.max_id do
	Controller.WorldSys.State.defaults.controllers[i] = Controller.Controller.defaults
end
Controller.WorldSys.Schema = Schema.AllOf(World.Sys.Schema, Schema.PartialObject{
	state = Controller.WorldSys.State.Schema,
})
function Controller.WorldSys:find(controller_id)
	if debug_checks_enabled then
		assert(Controller.WorldSys.Schema(self))
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
	end

	return self.state.controllers[controller_id]
end
function Controller.WorldSys:get_default()
	if debug_checks_enabled then
		assert(Controller.WorldSys.Schema(self))
	end

	return self:find(self.default_id)
end
function Controller.WorldSys:get_held(controller_id, input_name)
	if debug_checks_enabled then
		assert(Controller.WorldSys.Schema(self))
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
		assert(Controller.InputName.Schema(input_name))
	end

	return self.state.controllers[controller_id].inputs[input_name].held
end
function Controller.WorldSys:get_dir_x(controller_id)
	if debug_checks_enabled then
		assert(Controller.WorldSys.Schema(self))
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
	end

	local dir_x = 0
	if self:get_held(1, Controller.InputName.left) then
		dir_x = dir_x - 1
	end
	if self:get_held(1, Controller.InputName.right) then
		dir_x = dir_x + 1
	end

	return dir_x
end
function Controller.WorldSys:get_dir_y(controller_id)
	if debug_checks_enabled then
		assert(Controller.WorldSys.Schema(self))
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
	end

	local dir_y = 0
	if self:get_held(1, Controller.InputName.up) then
		dir_y = dir_y - 1
	end
	if self:get_held(1, Controller.InputName.down) then
		dir_y = dir_y + 1
	end

	return dir_y
end
function Controller.WorldSys:get_dirs(controller_id)
	if debug_checks_enabled then
		assert(Controller.WorldSys.Schema(self))
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
	end

	return self:get_dir_x(controller_id), self:get_dir_y(controller_id)
end
function Controller.WorldSys:get_toggled(controller_id, input_name)
	if debug_checks_enabled then
		assert(Controller.WorldSys.Schema(self))
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
		assert(Controller.InputName.Schema(input_name))
	end

	return self.sim.step_id == self.state.controllers[controller_id].inputs[input_name].step_id
end
function Controller.WorldSys:get_pressed(controller_id, input_name)
	if debug_checks_enabled then
		assert(Controller.WorldSys.Schema(self))
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
		assert(Controller.InputName.Schema(input_name))
	end

	return self:get_held(controller_id, input_name) and self:get_toggled(controller_id, input_name)
end
function Controller.WorldSys:get_released(controller_id, input_name)
	if debug_checks_enabled then
		assert(Controller.WorldSys.Schema(self))
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
		assert(Controller.InputName.Schema(input_name))
	end

	return not self:get_held(controller_id, input_name) and self:get_toggled(controller_id, input_name)
end
function Controller.WorldSys:on_init()
	Container.set_defaults(self.state, Controller.WorldSys.State.defaults)

	if debug_checks_enabled then
		assert(Controller.WorldSys.Schema(self))
	end
end
function Controller.WorldSys:on_input_set(controller_id, input_name, held)
	if debug_checks_enabled then
		assert(Controller.WorldSys.Schema(self))
		assert(Schema.BoundedInteger(1, Controller.Controller.max_id)(controller_id))
		assert(Controller.InputName.Schema(input_name))
		assert(Schema.Boolean(held))
	end

	local input = self.state.controllers[controller_id].inputs[input_name]
	input.held = held
	input.step_id = self.sim.step_id
end

Controller.GameSys = Game.Sys.new_metatable("controller")
Controller.GameSys.default_id = Controller.Controller.default_id
Controller.GameSys.State = {}
Controller.GameSys.State.Schema = Schema.Object{
	bindings = Schema.Object{
		controllers = Schema.BoundedArray(Controller.ControllerBindings.Schema, 1, Controller.Controller.max_id),
	},
}
Controller.GameSys.State.defaults = {
	bindings = {
		controllers = {},
	}
}
for controller_id = 1, Controller.Controller.max_id do
	Controller.GameSys.State.defaults.bindings.controllers[controller_id] = (
		Controller.ControllerBindings.defaults
	)
end
Controller.GameSys.State.defaults.bindings.controllers[Controller.Controller.default_id] = (
	Controller.ControllerBindings.defaults_default_controller
)
Controller.GameSys.Schema = Schema.AllOf(Game.Sys.Schema, Schema.PartialObject{
	state = Controller.GameSys.State.Schema,
	_world_game = Schema.Optional(World.GameSys.Schema),
})
function Controller.GameSys:on_init()
	Container.set_defaults(self.state, Controller.GameSys.State.defaults)

	self._world_game = self.sim:require(World.GameSys)
	self._world_game:require_world_sys(Controller.WorldSys)

	if debug_checks_enabled then
		assert(Controller.GameSys.Schema(self))
	end
end
function Controller.GameSys:handle_input_changes()
	if debug_checks_enabled then
		assert(Controller.GameSys.Schema(self))
	end

	local world = self._world_game.world
	if world == nil then
		return
	end

	local context = self.sim._context
	local controller_world = world:get(Controller.WorldSys)

	local controllers = controller_world.state.controllers
	for controller_id, controller in ipairs(controllers) do
		for input_name, input in pairs(controller.inputs) do
			local bindings = self.state.bindings.controllers[controller_id].inputs[input_name].bindings

			local held = false
			for _, binding in ipairs(bindings) do
				if binding.type == Controller.BindingType.keyboard then
					held = context ~= nil and context.window:get_key_state(binding.keyboard_key)
				elseif binding.type == Controller.BindingType.virtual then
					held = binding.virtual_value == true
				end

				if held then
					break
				end
			end

			if held ~= input.held then
				world:broadcast_pcall("on_input_set", controller_id, input_name, held)
			end
		end
	end
end
function Controller.GameSys:on_world_set()
	if debug_checks_enabled then
		assert(Controller.GameSys.Schema(self))
	end

	self:handle_input_changes()
end
function Controller.GameSys:on_step()
	if debug_checks_enabled then
		assert(Controller.GameSys.Schema(self))
	end

	self:handle_input_changes()
end

Controller.tests = Testing.add_suite("engine.controller", {
	run_world = function()
		local world = World.World.new()
		local controller_world = world:require(Controller.WorldSys)
		world:start()
		assert(Controller.Controller.Schema(controller_world:find(controller_world.default_id)))
		assert(Controller.Controller.Schema(controller_world:get_default()))
		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_dir_x(controller_world.default_id) == 0)
		assert(controller_world:get_dir_y(controller_world.default_id) == 0)

		world:step()
		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_dir_x(controller_world.default_id) == 0)
		assert(controller_world:get_dir_y(controller_world.default_id) == 0)

		controller_world:on_input_set(controller_world.default_id, Controller.InputName.up, true)
		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_dir_x(controller_world.default_id) == 0)
		assert(controller_world:get_dir_y(controller_world.default_id) == -1)

		world:step()
		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_dir_x(controller_world.default_id) == 0)
		assert(controller_world:get_dir_y(controller_world.default_id) == -1)

		controller_world:on_input_set(controller_world.default_id, Controller.InputName.up, false)
		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_dir_x(controller_world.default_id) == 0)
		assert(controller_world:get_dir_y(controller_world.default_id) == 0)

		controller_world:get_dirs(controller_world.default_id)

		world:finalize()
	end,
	run_game = function()
		local game = Game.Game.new()
		local controller_game = game:require(Controller.GameSys)
		local world_game = game:require(World.GameSys)
		game:start()

		local world = world_game.world
		local controller_world = world:get(Controller.WorldSys)

		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == false)

		game:step()

		local controllers = controller_game.state.bindings.controllers
		local controller = controllers[Controller.Controller.default_id]
		local up_bindings = controller.inputs[Controller.InputName.up].bindings
		up_bindings[#up_bindings + 1] = {type = "virtual", virtual_value = true}

		game:step()
		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == false)

		game:step()
		assert(controller_world:get_held(controller_world.default_id, Controller.InputName.up) == true)
		assert(controller_world:get_toggled(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_pressed(controller_world.default_id, Controller.InputName.up) == false)
		assert(controller_world:get_released(controller_world.default_id, Controller.InputName.up) == false)

		game:stop()
		game:finalize()
	end
})

return Controller
