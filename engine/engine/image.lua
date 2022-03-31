local Debugging = require("engine/core/debugging")
local Testing = require("engine/core/testing")
local Schema = require("engine/core/Schema")
local Container = require("engine/core/container")
local Model = require("engine/core/model")
local Sim = require("engine/engine/sim")
local Client = require("engine/engine/client")
local World = require("engine/engine/world")
local Game = require("engine/engine/game")
local Entity = require("engine/engine/entity")

local debug_checks_enabled = Debugging.debug_checks_enabled
local expensive_debug_checks_enabled = Debugging.expensive_debug_checks_enabled

local Image = {}

Image.FileType = Model.Enum("png")

Image.Image = {}
Image.Image.Schema = Schema.Object{
	filename = Schema.String,
	file_type = Image.FileType.Schema,
	u = Schema.NonNegativeInteger,
	v = Schema.NonNegativeInteger,
	width = Schema.NonNegativeInteger,
	height = Schema.NonNegativeInteger,
}

Image.Entity = {}
Image.Entity.Schema = Schema.AllOf(Entity.Entity.Schema, Schema.PartialObject{
	image_name = Schema.Optional(Schema.LabelString),
})

Image.Allocation = {}
Image.Allocation.Schema = Schema.Object{
	region_id = Schema.PositiveInteger,
	u = Schema.NonNegativeInteger,
	v = Schema.NonNegativeInteger,
	width = Schema.NonNegativeInteger,
	height = Schema.NonNegativeInteger,
}

Image.Allocator = {}
Image.Allocator.__index = Image.Allocator
Image.Allocator.Schema = Schema.Object{
	atlas = Schema.Optional(Client.Wrappers.Schema("TextureAtlas")),
	allocation_by_filename = Schema.Mapping(Schema.String, Image.Allocation.Schema),
}
function Image.Allocator.new(atlas)
	if debug_checks_enabled then
		assert(Client.Wrappers.Schema("TextureAtlas")(atlas))
	end
	local allocator = {
		atlas = atlas,
		allocation_by_filename = {},
	}
	setmetatable(allocator, Image.Allocator)

	if expensive_debug_checks_enabled then
		assert(Image.Allocator.Schema(allocator))
	end

	return allocator
end
function Image.Allocator:load(filename, file_type)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.Allocator.Schema(self))
		end
		assert(Schema.String(filename))
		assert(Schema.Optional(Image.FileType.Schema)(file_type))
	end

	file_type = file_type or Image.FileType.png

	local region = self.allocation_by_filename[filename]
	if region ~= nil then
		return region
	end

	local region_id = self.atlas:get_count() + 1
	local u1, v1, u2, v2

	if file_type == Image.FileType.png then
		u1, v1, u2, v2 = self.atlas:set_region_png_file{
			id = region_id,
			filename = filename,
		}
	else
		error("Unknown file_type "..file_type)
	end

	region = {
		region_id = region_id,
		u = u1,
		v = v1,
		width = u2 - u1,
		height = v2 - v1,
	}
	self.allocation_by_filename[filename] = region

	if expensive_debug_checks_enabled then
		assert(Image.Allocator.Schema(self))
		assert(Image.Allocation.Schema(region))
	end

	return region
end
function Image.Allocator:find(filename)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.Allocator.Schema(self))
		end
		assert(Schema.String(filename))
	end

	return self.allocation_by_filename[filename]
end
function Image.Allocator:get_image_bounds(image)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.Allocator.Schema(self))
			assert(Image.Image.Schema(image))
		end
	end

	local allocation = self:find(image.filename)
	local u, v = allocation.u + image.u, allocation.v + image.v
	return {u, v, u + image.width, v + image.height}
end

Image.WorldSys = World.Sys.new_metatable("image")
Image.WorldSys.State = {}
Image.WorldSys.State.Schema = Schema.Object{
	images = Schema.Mapping(Schema.LabelString, Image.Image.Schema),
}
Image.WorldSys.State.defaults = {
	images = {},
}
Image.WorldSys.Schema = Schema.AllOf(World.Sys.Schema, Schema.PartialObject{
	state = Image.WorldSys.State.Schema,
	_allocator = Schema.Optional(Image.Allocator.Schema),
	_client_world = Client.WorldSys.Schema,
	_entity_world = Entity.WorldSys.Schema,
	_image_bounds = Schema.Mapping(Schema.LabelString, Schema.BoundedArray(Schema.Integer, 4, 4)),
})
function Image.WorldSys:index(image_name, image)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.WorldSys.Schema(self))
			assert(Schema.Optional(Image.Image.Schema)(image))
		end
		assert(Schema.LabelString(image_name))
	end

	image = image or self.state.images[image_name]

	self.state.images[image_name] = image

	if self._allocator ~= nil then
		self._allocator:load(image.filename, image.file_type)

		self._image_bounds[image_name] = self._allocator:get_image_bounds(image)
	end
end
function Image.WorldSys:index_all()
	if expensive_debug_checks_enabled then
		assert(Image.WorldSys.Schema(self))
	end

	Container.set_defaults(self.state, Image.WorldSys.State.defaults)

	self._image_bounds = {}

	for image_name, image in pairs(self.state.images) do
		self:index(image_name, image)
	end
end
function Image.WorldSys:set(image_name, image)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.WorldSys.Schema(self))
			assert(Image.Image.Schema(image))
		end
		assert(Schema.LabelString(image_name))
	end

	self:index(image_name, image)
	return image
end
function Image.WorldSys:set_batch(name_bounds_map, filename, file_type, grid_size)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.WorldSys.Schema(self))
			assert(Schema.Mapping(Schema.String, Schema.BoundedArray(Schema.Integer, 2, 4))(name_bounds_map))
		end
		assert(Schema.String(filename))
		assert(Schema.Optional(Image.FileType.Schema)(file_type))
		assert(Schema.Optional(Schema.NonNegativeInteger)(grid_size))
	end

	for image_name, image_bounds in pairs(name_bounds_map) do
		self:set(image_name, {
			u = image_bounds[1],
			v = image_bounds[2],
			width = image_bounds[3] or grid_size,
			height = image_bounds[4] or grid_size,
			filename = filename,
			file_type = file_type or Image.FileType.png,
		})
	end
end
function Image.WorldSys:find(image_name)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.WorldSys.Schema(self))
		end
		assert(Schema.LabelString(image_name))
	end

	return self.state.images[image_name]
end
function Image.WorldSys:entity_index(entity_id, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.WorldSys.Schema(self))
			assert(Schema.Optional(Image.Entity.Schema)(entity))
		end
		assert(Schema.PositiveInteger(entity_id))
	end

	entity = entity or self._entity_world:find(entity_id)

	local entity_index = self._entity_world:get_entity_index()

	local image_name = entity.image_name
	if image_name ~= nil and self._allocator ~= nil then
		local uv_bounds = self._image_bounds[image_name]

		entity_index:set_sprite(
			entity_id,
			uv_bounds[1], uv_bounds[2], uv_bounds[3], uv_bounds[4],
			entity.r or 255, entity.g or 255, entity.b or 255, entity.a or 255,
			entity.z or 0,
			-- transform
			entity.scale_x or 1, 0, 0, 0,
			0, entity.scale_y or 1, 0, 0,
			0, 0, 1, 0,
			entity.translate_x or 0, entity.translate_y or 0, 0, 1
		)
	else
		entity_index:set_sprite(
			entity_id,
			0, 0, 0, 0,  -- uv bounds
			0, 0, 0, 0,  -- color
			0, -- depth
			-- transform
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		)
	end
end
function Image.WorldSys:entity_set_image_name(entity_id, image_name, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.WorldSys.Schema(self))
			assert(Schema.Optional(Image.Entity.Schema)(entity))
		end
		assert(Schema.PositiveInteger(entity_id))
		assert(Schema.Optional(Schema.LabelString)(image_name))
		assert(image_name == nil or self.state.images[image_name] ~= nil)

		assert(self.sim.status == Sim.Status.started)
	end

	entity = entity or self._entity_world:find(entity_id)

	entity.image_name = image_name

	self:entity_index(entity_id, entity)
end
function Image.WorldSys:entity_unset(entity_id, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.WorldSys.Schema(self))
			assert(Schema.Optional(Image.Entity.Schema)(entity))
		end
		assert(Schema.PositiveInteger(entity_id))

		assert(self.sim.status == Sim.Status.started)
	end

	self:entity_set_image_name(entity_id, nil, entity)
end
function Image.WorldSys:draw(image_name, x, y, width, height, r, g, b, a, z)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.WorldSys.Schema(self))
		end
		assert(Schema.LabelString(image_name))
		assert(Schema.Integer(x))
		assert(Schema.Integer(y))
		assert(Schema.NonNegativeInteger(width))
		assert(Schema.NonNegativeInteger(height))
		assert(Schema.Optional(Schema.BoundedInteger)(r, 0, 255))
		assert(Schema.Optional(Schema.BoundedInteger)(g, 0, 255))
		assert(Schema.Optional(Schema.BoundedInteger)(b, 0, 255))
		assert(Schema.Optional(Schema.BoundedInteger)(a, 0, 255))
		assert(Schema.Optional(Schema.Number)(z))

		assert(self.sim.status == Sim.Status.started)
	end

	local vertex_array = self._client_world:get_vertex_array()
	local uv_bounds = self._image_bounds[image_name]

	vertex_array:add_sprite(
		x, y, x + width, y + height,
		uv_bounds[1], uv_bounds[2], uv_bounds[3], uv_bounds[4],
		r or 255, g or 255, b or 255, a or 255,
		z
	)
end
function Image.WorldSys:on_init()
	local game = self.sim._game
	if game ~= nil then
		local image_game = game:get(Image.GameSys)
		if image_game ~= nil then
			self._allocator = image_game.allocator
		end
	end

	Container.set_defaults(self.state, Image.WorldSys.State.defaults)

	self._image_bounds = {}

	self._client_world = self.sim:require(Client.WorldSys)
	self._entity_world = self.sim:require(Entity.WorldSys)

	self:index_all()

	if expensive_debug_checks_enabled then
		assert(Image.WorldSys.Schema(self))
	end
end
function Image.WorldSys:on_draw()
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.WorldSys.Schema(self))
		end

		assert(self.sim.status == Sim.Status.started)
	end

	local vertex_array = self._client_world:get_vertex_array()
	local entity_index = self._entity_world:get_entity_index()
	entity_index:add_to_vertex_array{vertex_array = vertex_array}
end
function Image.WorldSys:on_entity_index(entity_id, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.WorldSys.Schema(self))
			assert(Image.Entity.Schema(entity))
		end
		assert(Schema.PositiveInteger(entity_id))
	end

	self:entity_index(entity_id, entity)
end

Image.GameSys = Game.Sys.new_metatable("image")
Image.GameSys.Schema = Schema.AllOf(Game.Sys.Schema, Schema.PartialObject{
	allocator = Schema.Optional(Image.Allocator.Schema),
})
function Image.GameSys:load(filename, file_type)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.GameSys.Schema(self))
		end
		assert(Schema.String(filename))
		assert(Schema.Optional(Image.FileType.Schema)(file_type))
	end

	if self.allocator == nil then
		return 0, 0, 0, 0
	end

	local region = self.allocator:load(filename, file_type)
	return region.u, region.v, region.width, region.height
end
function Image.GameSys:on_init()
	local client_game = self.sim:require(Client.GameSys)
	local world_game = self.sim:require(World.GameSys)
	world_game:require_world_sys(Image.WorldSys)
	world_game:require_world_sys(Entity.WorldSys)

	if client_game.context ~= nil then
		self.allocator = Image.Allocator.new(client_game.context.texture_atlas)
	end

	if expensive_debug_checks_enabled then
		assert(Image.GameSys.Schema(self))
	end
end

Image.tests = Testing.add_suite("engine.entity", {
	run_game = function()
		local game = Game.Game.new({client = {visible = false}})
		-- local game = Game.Game.new()  -- to test with a non-headless client

		local WorldTestSys = World.Sys.new_metatable("world")
		function WorldTestSys:on_init()
			self._image_world = self.sim:require(Image.WorldSys)
		end
		function WorldTestSys:on_draw()
			self._image_world:draw("wall", 32, 32, 128, 128)
		end

		local image_game = game:require(Image.GameSys)
		local world_game = game:require(World.GameSys)
		world_game:require_world_sys(WorldTestSys)

		local filename = "./examples/engine_test/data/sprites.png"
		local file_type = "png"
		local grid_size = 8
		image_game:load(filename)
		game:start()

		local world = world_game.world
		local image_world = world:get(Image.WorldSys)
		local entity_world = world:get(Entity.WorldSys)
		image_world:set(
			"hello",
			{filename = filename, file_type = file_type, u = 0, v = 0, width = grid_size, height = grid_size})
		image_world:index("hello")

		local name_image_map = {
			blank = {0, 0},
			none = {8, 0},
			wall = {16, 24},
		}
		image_world:set_batch(name_image_map, filename, file_type, grid_size)
		Container.assert_equal(
			image_world:find("blank"),
			{filename = filename, file_type = file_type, u = 0, v = 0, width = grid_size, height = grid_size})
		Container.assert_equal(
			image_world:find("none"),
			{filename = filename, file_type = file_type, u = 8, v = 0, width = grid_size, height = grid_size})


		local entity_id
		for i = 1, 15 do
			entity_id = entity_world:add{
				x = i * grid_size,
				y = grid_size,
				width = grid_size,
				height = grid_size,
				image_name = "blank",
			}
			image_world:entity_index(entity_id)
			image_world:entity_set_image_name(entity_id, "wall")
			image_world:index_all()
			game:step()
		end

		image_world:entity_unset(entity_id)

		game:stop()
		game:finalize()
	end,
})

return Image
