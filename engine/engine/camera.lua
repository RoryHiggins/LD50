local Debugging = require("engine/core/debugging")
local Schema = require("engine/core/Schema")
local Container = require("engine/core/container")
local Testing = require("engine/core/testing")
local Sim = require("engine/engine/sim")
local Game = require("engine/engine/game")
local World = require("engine/engine/world")

local debug_checks_enabled = Debugging.debug_checks_enabled

local Camera = {}

Camera.Model = {}
Camera.Model.Camera = {}
Camera.Model.Camera.Schema = Schema.Object{
	transform = Schema.Optional(Schema.Object{
		translate_x = Schema.Optional(Schema.Number),
		translate_y = Schema.Optional(Schema.Number),
		scale_x = Schema.Optional(Schema.PositiveNumber),
		scale_y = Schema.Optional(Schema.PositiveNumber),
		rotate_z = Schema.Optional(Schema.Number),
	}),
	viewport = Schema.Optional(Schema.Object{
		x = Schema.Number,
		y = Schema.Number,
		width = Schema.PositiveNumber,
		height = Schema.PositiveNumber,
	}),
}
Camera.Model.Camera.defaults = {
	transform = {
		translate_x = 0,
		translate_y = 0,
	}
}
function Camera.Model.Camera.new(translate_x, translate_y)
	assert(Schema.Optional(Schema.Number)(translate_x))
	assert(Schema.Optional(Schema.Number)(translate_y))

	return Container.update({}, Camera.Model.Camera.defaults, {
		transform = {
			translate_x = translate_x,
			translate_y = translate_y,
		}
	})
end

Camera.Sys = Sim.Sys.new_metatable("camera")
Camera.Sys.default_camera_name = "default"
Camera.Sys.State = {}
Camera.Sys.State.Schema = Schema.Object{
	cameras = Schema.Mapping(Schema.LabelString, Camera.Model.Camera.Schema),
}
Camera.Sys.State.defaults = {
	cameras = {
		default_camera_name = Camera.Model.Camera.defaults,
	},
}
function Camera.Sys:on_init()
	Container.set_defaults(self.state, Camera.Sys.State.defaults)

	if debug_checks_enabled then
		assert(Camera.Sys.State.Schema(self.state))
	end

	if self.state.cameras[Camera.Sys.default_camera_name] == nil then
		self:set(Camera.Sys.default_camera_name, Camera.Model.Camera.defaults)
	end
end
function Camera.Sys:on_world_init()
	self.sim:get(World.GameSys).world:require(Camera.Sys)
end
function Camera.Sys:set(name, camera)
	if debug_checks_enabled then
		assert(Schema.LabelString(name))
	end

	self.state.cameras[name] = camera

	if debug_checks_enabled then
		assert(Camera.Sys.State.Schema(self.state))
	end
end
function Camera.Sys:find(name)
	if debug_checks_enabled then
		assert(Schema.LabelString(name))
		assert(self.state.cameras[name] ~= nil)
	end

	return self.state.cameras[name]
end
function Camera.Sys:get_default()
	return self:find(Camera.Sys.default_camera_name)
end

Camera.tests = Testing.add_suite("engine.Camera", {
	run = function()
		local game_sim = Game.Game.new()
		local world_sim = game_sim:require(World.GameSys)
		local camera_sys = game_sim:require(Camera.Sys)
		game_sim:start()

		camera_sys:set("blah", camera_sys:get_default())
		camera_sys:set("blah2", Camera.Model.Camera.new(1, 2))
		assert(camera_sys:find("blah2").transform.translate_y == 2)

		assert(Camera.Model.Camera.Schema(world_sim.world:get(Camera.Sys):get_default()))

		game_sim:step()
		game_sim:finalize()
	end
})

return Camera
