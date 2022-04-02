local Debugging = require("engine/core/debugging")
local Testing = require("engine/core/testing")
local Logging = require("engine/core/logging")
local Math = require("engine/core/math")
local Schema = require("engine/core/schema")
local Client = require("engine/engine/client")
local World = require("engine/engine/world")
local Camera = require("engine/engine/camera")
local Text = require("engine/engine/text")
local Game = require("engine/engine/game")
local Entity = require("engine/engine/entity")
local Image = require("engine/engine/image")

local debug_checks_enabled = Debugging.debug_checks_enabled
local expensive_debug_checks_enabled = Debugging.expensive_debug_checks_enabled

local CameraTarget = {}

CameraTarget.Entity = {}
CameraTarget.Entity.Schema = Schema.AllOf(Entity.Entity.Schema, Schema.PartialObject{
	camera_name = Schema.Optional(Schema.LabelString),
	camera_speed = Schema.Optional(Schema.PositiveNumber),
})

CameraTarget.WorldSys = World.Sys.new_metatable("camera_target")
CameraTarget.WorldSys.Schema = Schema.AllOf(World.Sys.Schema, Schema.PartialObject{
	_client_world = Client.WorldSys.Schema,
	_camera_world = Camera.WorldSys.Schema,
	_entity_world = Entity.WorldSys.Schema,
})
function CameraTarget.WorldSys:entity_set(entity_id, camera_name, speed, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(CameraTarget.WorldSys.Schema(self))
			assert(Schema.Optional(CameraTarget.Entity.Schema)(entity))
		end
		assert(Schema.Optional(Schema.LabelString)(camera_name))
		assert(Schema.PositiveInteger(entity_id))
		assert(Schema.Optional(Schema.PositiveNumber)(speed))
	end

	entity = entity or self._entity_world:find(entity_id)

	entity.camera_name = camera_name
	if camera_name == nil then
		entity.camera_speed = nil
		self._entity_world:untag(entity_id, {self.sys_name}, entity)
	else
		entity.camera_speed = speed
		self._entity_world:tag(entity_id, {self.sys_name}, entity)
	end
end
function CameraTarget.WorldSys:entity_set_default_camera(entity_id, speed, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(CameraTarget.WorldSys.Schema(self))
		end
		assert(Schema.PositiveInteger(entity_id))
		assert(Schema.Optional(Schema.PositiveNumber)(speed))
	end

	entity = entity or self._entity_world:find(entity_id)

	self:entity_set(entity_id, self._camera_world.default_camera_name, speed, entity)
end
function CameraTarget.WorldSys:entity_unset(entity_id, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(CameraTarget.WorldSys.Schema(self))
		end
		assert(Schema.PositiveInteger(entity_id))
	end

	entity = entity or self._entity_world:find(entity_id)

	self:entity_set(entity_id, nil, nil, entity)
end
function CameraTarget.WorldSys:on_init()
	self._client_world = self.sim:require(Client.WorldSys)
	self._camera_world = self.sim:require(Camera.WorldSys)
	self._entity_world = self.sim:require(Entity.WorldSys)

	if expensive_debug_checks_enabled then
		assert(CameraTarget.WorldSys.Schema(self))
	end
end
function CameraTarget.WorldSys:on_step_end()
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(CameraTarget.WorldSys.Schema(self))
		end
	end

	local target_entities = self._entity_world:get_all_tagged_array(self.sys_name)

	if expensive_debug_checks_enabled then
		assert(Schema.Array(CameraTarget.Entity.Schema)(target_entities))
	end

	local seen_cameras
	if debug_checks_enabled then
		seen_cameras = {}
	end

	for _, entity in ipairs(target_entities) do
		local camera_name = entity.camera_name or self._camera_world.default_camera_name
		local camera = self._camera_world:find(camera_name)

		if debug_checks_enabled then
			assert(camera ~= nil)
		end

		if camera ~= nil then
			if debug_checks_enabled then
				if seen_cameras[camera_name] == true then
					Logging.warning("multiple entities attached to camera_name=%s", camera_name)
				end
				seen_cameras[camera_name] = true
			end

			local target_x = (entity.x or 0) + ((entity.width or 0) / 2)
			local target_y = (entity.y or 0) + ((entity.height or 0) / 2)
			local orig_x, orig_y = self._client_world:camera_get_orig(camera_name)

			local speed = entity.camera_speed
			if speed ~= nil then
				orig_x, orig_y = Math.move_towards(orig_x, orig_y, target_x, target_y, speed)
			else
				orig_x, orig_y = target_x, target_y
			end

			self._client_world:camera_set_orig(camera_name, orig_x, orig_y)
		end
	end
end

CameraTarget.GameSys = Game.Sys.new_metatable("camera_target")
CameraTarget.GameSys.WorldSys = CameraTarget.WorldSys

CameraTarget.tests = Testing.add_suite("engine.camera_target", {
	run_game = function()
		local game = Game.Game.new({client = {visible = false}})
		-- local game = Game.Game.new()  -- to test with a non-headless client

		local WorldTestSys = World.Sys.new_metatable("world")
		function WorldTestSys:on_init()
			self._text_world = self.sim:require(Text.WorldSys)
		end
		function WorldTestSys:on_draw()
			self._text_world:draw("default", "wow", 0, 0)
		end

		game:require(Client.GameSys)
		game:require(Image.GameSys)

		local world_game = game:require(World.GameSys)
		world_game:require_world_sys(CameraTarget.WorldSys)
		world_game:require_world_sys(WorldTestSys)

		game:start()

		local world = world_game.world
		local entity_world = world:get(Entity.WorldSys)
		local camera_target_world = world:get(CameraTarget.WorldSys)

		local entity_id, entity = entity_world:add{x = 0, y = 0}
		camera_target_world:entity_set_default_camera(entity_id)

		for _ = 1, 6 do
			game:step()
			entity_world:set_pos(entity_id, entity.x + 1, entity.y + 1)
		end

		game:stop()
		game:finalize()
	end,
})

return CameraTarget
