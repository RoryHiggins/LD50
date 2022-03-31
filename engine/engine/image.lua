local Shim = require("engine/core/shim")
local Math = require("engine/core/math")
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
	u = Schema.NonNegativeInteger,
	v = Schema.NonNegativeInteger,
	width = Schema.NonNegativeInteger,
	height = Schema.NonNegativeInteger,
	filename = Schema.String,
	file_type = Schema.Optional(Image.FileType.Schema),
}

Image.Entity = {}
Image.Entity.Schema = Schema.AllOf(Entity.Entity.Schema, Schema.PartialObject{
	image_name = Schema.Optional(Schema.LabelString),
	z = Schema.Optional(Schema.Number),
	r = Schema.Optional(Schema.BoundedInteger(0, 255)),
	g = Schema.Optional(Schema.BoundedInteger(0, 255)),
	b = Schema.Optional(Schema.BoundedInteger(0, 255)),
	a = Schema.Optional(Schema.BoundedInteger(0, 255)),
	translate_x = Schema.Optional(Schema.Number),
	translate_y = Schema.Optional(Schema.Number),
	scale_x = Schema.Optional(Schema.BoundedNumber(Math.epsilon, Math.integer_max)),
	scale_y = Schema.Optional(Schema.BoundedNumber(Math.epsilon, Math.integer_max)),
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
	_image_name_bounds_cache = Schema.Mapping(Schema.LabelString, Schema.BoundedArray(Schema.Integer, 4, 4)),
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

		self._image_name_bounds_cache[image_name] = self._allocator:get_image_bounds(image)
	end
end
function Image.WorldSys:index_all()
	if expensive_debug_checks_enabled then
		assert(Image.WorldSys.Schema(self))
	end

	Container.set_defaults(self.state, Image.WorldSys.State.defaults)

	self._image_name_bounds_cache = {}

	for image_name, image in pairs(self.state.images) do
		self:index(image_name, image)
	end
end
function Image.WorldSys:add(image_name, image)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.WorldSys.Schema(self))
			assert(Image.Image.Schema(image))
		end
		assert(Schema.LabelString(image_name))
	end

	if self.state.images[image_name] ~= nil then
		return
	end

	self:index(image_name, image)
end
function Image.WorldSys:add_batch(name_bounds_map, filename, file_type, grid_size)
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
		self:add(image_name, {
			u = image_bounds[1],
			v = image_bounds[2],
			width = image_bounds[3] or grid_size,
			height = image_bounds[4] or grid_size,
			filename = filename,
			file_type = file_type,
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
			assert(Image.Entity.Schema(entity))
		end
		assert(Schema.PositiveInteger(entity_id))
	end

	local entity_index = self._entity_world:get_entity_index()

	local image_name = entity.image_name
	if image_name ~= nil and self._allocator ~= nil then
		local bounds_cache = self._image_name_bounds_cache
		local bounds = bounds_cache[image_name]
		if bounds == nil then
			local image = self:find(image_name)
			bounds = self._allocator:get_image_bounds(image)
			bounds_cache[image_name] = bounds
		end

		entity_index:set_sprite(
			entity_id,
			bounds[1], bounds[2], bounds[3], bounds[4],
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
			assert(Image.Entity.Schema(entity))
		end
		assert(Schema.PositiveInteger(entity_id))
		assert(Schema.LabelString(image_name))
		assert(self.state.images[image_name] ~= nil)
	end

	entity.image_name = image_name
	self:entity_index(entity_id, entity)
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

	self._image_name_bounds_cache = {}

	self._client_world = self.sim:require(Client.WorldSys)
	self._entity_world = self.sim:require(Entity.WorldSys)

	self:index_all()

	if expensive_debug_checks_enabled then
		assert(Image.WorldSys.Schema(self))
	end
end
function Image.WorldSys:on_draw()
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
		-- local game = Game.Game.new()
		local image_game = game:require(Image.GameSys)
		local world_game = game:require(World.GameSys)

		local filename = "./examples/engine_test/data/sprites.png"
		local file_type = "png"
		local grid_size = 8
		image_game:load(filename)
		game:start()

		local world = world_game.world
		local image_world = world:get(Image.WorldSys)
		local entity_world = world:get(Entity.WorldSys)
		image_world:add(
			"hello", {filename = filename, file_type = "png", u = 0, v = 0, width = grid_size, height = grid_size})

		local name_image_map = {
			blank = {0, 0},
			none = {8, 0},
			wall = {16, 24},
		}
		image_world:add_batch(name_image_map, filename, file_type, grid_size)
		Container.assert_equal(
			image_world:find("blank"),
			{filename = filename, file_type = "png", u = 0, v = 0, width = grid_size, height = grid_size})
		Container.assert_equal(
			image_world:find("none"),
			{filename = filename, file_type = "png", u = 8, v = 0, width = grid_size, height = grid_size})

		for i = 1, 30 do
			entity_world:add{
				x = i * grid_size,
				y = grid_size,
				width = grid_size,
				height = grid_size,
				image_name = "wall",
			}
			game:step()
		end

		game:stop()
		game:finalize()
	end,
})

return Image
