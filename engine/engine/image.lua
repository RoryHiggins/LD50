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
function Image.Allocator:load_png(filename)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.Allocator.Schema(self))
		end
		assert(Schema.String(filename))
	end

	local region = self.allocation_by_filename[filename]
	if region ~= nil then
		return region
	end

	local region_id = self.atlas:get_count() + 1
	local u1, v1, u2, v2 = self.atlas:set_region_png_file{
		id = region_id,
		filename = filename,
	}
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

Image.WorldSys = World.Sys.new_metatable("image")
Image.WorldSys.Schema = Schema.AllOf(World.Sys.Schema, Schema.PartialObject{
	_allocator = Schema.Optional(Image.Allocator.Schema),
})
function Image.WorldSys:init()
	local game = self.sim._game
	if game ~= nil then
		local image_game = game:get(Image.GameSys)
		if image_game ~= nil then
			self._allocator = image_game.allocator
		end
	end
end

Image.GameSys = Game.Sys.new_metatable("image")
Image.GameSys.Schema = Schema.AllOf(Game.Sys.Schema, Schema.PartialObject{
	allocator = Schema.Optional(Image.Allocator.Schema),
})
function Image.GameSys:load_png(filename)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Image.GameSys.Schema(self))
		end

		assert(Schema.String(filename))
	end

	if self.allocator == nil then
		return 0, 0, 0, 0
	end

	local region = self.allocator:load_png(filename)
	return region.u, region.v, region.width, region.height
end
function Image.GameSys:on_init()
	local client_game = self.sim:require(Client.GameSys)
	self.sim:require(World.GameSys):require_world_sys(Image.WorldSys)

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
		local image_game = game:require(Image.GameSys)

		image_game:load_png("./examples/engine_test/data/sprites.png")
		game:start()

		game:step()
		game:stop()
		game:finalize()
	end,
})

return Image
