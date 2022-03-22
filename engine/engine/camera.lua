local Debugging = require("engine/core/debugging")
local Schema = require("engine/core/Schema")
local Container = require("engine/core/container")
local Testing = require("engine/core/testing")
local World = require("engine/engine/world")

local debug_checks_enabled = Debugging.debug_checks_enabled

local Camera = {}

Camera.Model = {}
Camera.Model.Camera = {}
Camera.Model.Camera.Schema = Schema.Object{
	transform = Schema.Object{
		translate_x = Schema.Number,
		translate_y = Schema.Number,
		translate_z = Schema.Number,
		scale_x = Schema.Optional(Schema.PositiveNumber),
		scale_y = Schema.Optional(Schema.PositiveNumber),
		scale_z = Schema.Optional(Schema.PositiveNumber),
		rotate_z = Schema.Optional(Schema.Number),
	},
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
		translate_z = 0,
	},
}
function Camera.Model.Camera.new_ortho_2d(x, y, z)
	if debug_checks_enabled then
		assert(Schema.Optional(Schema.Number)(x))
		assert(Schema.Optional(Schema.Number)(y))
		assert(Schema.Optional(Schema.Number)(z))
	end

	return Container.update({}, Camera.Model.Camera.defaults, {
		transform = {
			translate_x = x,
			translate_y = y,
			translate_z = z,
		}
	})
end

Camera.WorldSys = World.Sys.new_metatable("camera")
Camera.WorldSys.default_name = "default"
Camera.WorldSys.State = {}
Camera.WorldSys.State.Schema = Schema.Object{
	cameras = Schema.Mapping(Schema.LabelString, Camera.Model.Camera.Schema),
}
Camera.WorldSys.State.defaults = {
	cameras = {
		[Camera.WorldSys.default_name] = Camera.Model.Camera.defaults,
	},
}
function Camera.WorldSys:on_init()
	Container.set_defaults(self.state, Camera.WorldSys.State.defaults)

	if debug_checks_enabled then
		assert(Camera.WorldSys.State.Schema(self.state))
	end
end
function Camera.WorldSys:set(name, camera)
	if debug_checks_enabled then
		assert(Schema.LabelString(name))
	end

	self.state.cameras[name] = camera

	if debug_checks_enabled then
		assert(Camera.WorldSys.State.Schema(self.state))
	end
end
function Camera.WorldSys:set_pos(name, x, y, z)
	if debug_checks_enabled then
		assert(Schema.LabelString(name))
		assert(Schema.Optional(Schema.Number)(x))
		assert(Schema.Optional(Schema.Number)(y))
		assert(Schema.Optional(Schema.Number)(z))
	end

	self:set(name, Camera.Model.Camera.new_ortho_2d(x, y, z))
end
function Camera.WorldSys:find(name)
	if debug_checks_enabled then
		assert(Schema.LabelString(name))
		assert(self.state.cameras[name] ~= nil)
	end

	return self.state.cameras[name]
end
local function order_camera_depth_sorted(a, b)
	-- return in draw order (bottom to top, aka max z value first)
	return a.transform.translate_z > b.transform.translate_z
end
function Camera.WorldSys:all_depth_ordered()
	local cameras = Container.get_values(self.state.cameras)
	table.sort(cameras, order_camera_depth_sorted)
	return cameras
end
function Camera.WorldSys:get_default()
	return self:find(Camera.WorldSys.default_name)
end

Camera.tests = Testing.add_suite("engine.Camera", {
	run = function()
		local world_sim = World.World.new()
		local camera_sys = world_sim:require(Camera.WorldSys)
		world_sim:start()

		camera_sys:set("blah", Camera.Model.Camera.defaults)
		camera_sys:set("blah2", Camera.Model.Camera.new_ortho_2d(1, 2, 3))
		assert(camera_sys:find("blah2").transform.translate_y == 2)

		assert(Camera.Model.Camera.Schema(camera_sys:find("blah2")))

		world_sim:step()
		world_sim:finalize()
	end
})

return Camera
