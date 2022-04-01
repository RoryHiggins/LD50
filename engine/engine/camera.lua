local Debugging = require("engine/core/debugging")
local Schema = require("engine/core/schema")
local Container = require("engine/core/container")
local Testing = require("engine/core/testing")
local World = require("engine/engine/world")

local debug_checks_enabled = Debugging.debug_checks_enabled

local Camera = {}

Camera.Camera = {}
Camera.Camera.Schema = Schema.Object{
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
Camera.Camera.defaults = {
	transform = {
		translate_x = 0,
		translate_y = 0,
		translate_z = 0,
	},
}
function Camera.Camera.new_ortho_2d(x, y, z)
	if debug_checks_enabled then
		assert(Schema.Optional(Schema.Number)(x))
		assert(Schema.Optional(Schema.Number)(y))
		assert(Schema.Optional(Schema.Number)(z))
	end

	return Container.update({}, Camera.Camera.defaults, {
		transform = {
			translate_x = -x,
			translate_y = -y,
			translate_z = -z,
		}
	})
end

Camera.WorldSys = World.Sys.new_metatable("camera")
Camera.WorldSys.default_camera_name = "default"
Camera.WorldSys.State = {}
Camera.WorldSys.State.Schema = Schema.Object{
	cameras = Schema.Mapping(Schema.LabelString, Camera.Camera.Schema),
}
Camera.WorldSys.State.defaults = {
	cameras = {
		[Camera.WorldSys.default_camera_name] = Camera.Camera.defaults,
	},
}
Camera.WorldSys.Schema = Schema.AllOf(World.Sys.Schema, Schema.PartialObject{
	state = Camera.WorldSys.State.Schema,
})
function Camera.WorldSys:on_init()
	Container.set_defaults(self.state, Camera.WorldSys.State.defaults)

	if debug_checks_enabled then
		assert(Camera.WorldSys.Schema(self))
	end
end
function Camera.WorldSys:set(name, camera)
	if debug_checks_enabled then
		assert(Camera.WorldSys.Schema(self))
		assert(Schema.LabelString(name))
		assert(Camera.Camera.Schema(camera))
	end

	self.state.cameras[name] = camera

	if debug_checks_enabled then
		assert(Camera.WorldSys.Schema(self))
	end
end
function Camera.WorldSys:set_pos(name, x, y, z)
	if debug_checks_enabled then
		assert(Camera.WorldSys.Schema(self))
		assert(Schema.LabelString(name))
		assert(Schema.Optional(Schema.Number)(x))
		assert(Schema.Optional(Schema.Number)(y))
		assert(Schema.Optional(Schema.Number)(z))
	end

	self:set(name, Camera.Camera.new_ortho_2d(x, y, z))
end
function Camera.WorldSys:find(name)
	if debug_checks_enabled then
		assert(Camera.WorldSys.Schema(self))
		assert(Schema.LabelString(name))
		assert(Camera.Camera.Schema(self.state.cameras[name]))
	end

	return self.state.cameras[name]
end
function Camera.WorldSys:get_pos(name)
	if debug_checks_enabled then
		assert(Camera.WorldSys.Schema(self))
		assert(Schema.LabelString(name))
		assert(Camera.Camera.Schema(self.state.cameras[name]))
	end

	local camera = self.state.cameras[name]
	if camera == nil then
		return 0, 0
	end

	local transform = camera.transform
	return -transform.translate_x, -transform.translate_y, -transform.translate_z
end
local function order_camera_depth_sorted(a, b)
	-- return in draw order (bottom to top, aka max z value first)
	return a.transform.translate_z > b.transform.translate_z
end
function Camera.WorldSys:all_depth_ordered()
	if debug_checks_enabled then
		assert(Camera.WorldSys.Schema(self))
	end

	local cameras = Container.get_values(self.state.cameras)
	table.sort(cameras, order_camera_depth_sorted)
	return cameras
end
function Camera.WorldSys:get_default()
	if debug_checks_enabled then
		assert(Camera.WorldSys.Schema(self))
	end

	return self:find(self.default_camera_name)
end

Camera.tests = Testing.add_suite("engine.camera", {
	run = function()
		local world_sim = World.World.new()
		local camera_world = world_sim:require(Camera.WorldSys)

		camera_world:set("blah", Camera.Camera.defaults)
		camera_world:set("blah", Camera.Camera.new_ortho_2d(1, 2, 3))
		assert(camera_world:find("blah").transform.translate_x == -1)
		assert(camera_world:find("blah").transform.translate_y == -2)
		assert(camera_world:find("blah").transform.translate_z == -3)
		assert(Camera.Camera.Schema(camera_world:find("blah")))

		camera_world:set_pos("blah2", 4, 5, 6)
		assert(camera_world:find("blah2").transform.translate_x == -4)
		assert(camera_world:find("blah2").transform.translate_y == -5)
		assert(camera_world:find("blah2").transform.translate_z == -6)
		assert(Camera.Camera.Schema(camera_world:find("blah2")))

		assert(Camera.Camera.Schema(camera_world:get_default(camera_world.default_id)))

		world_sim:start()
		world_sim:step()
		world_sim:finalize()
	end
})

return Camera
