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
local Image = require("engine/engine/image")

local debug_checks_enabled = Debugging.debug_checks_enabled
local expensive_debug_checks_enabled = Debugging.expensive_debug_checks_enabled

local Text = {}

Text.FontType = Model.Enum("ascii")

Text.Font = {}
Text.Font.Schema = Schema.Object{
	filename = Schema.String,
	file_type = Image.FileType.Schema,
	font_type = Text.FontType.Schema,
	u = Schema.NonNegativeInteger,
	v = Schema.NonNegativeInteger,
	width = Schema.NonNegativeInteger,
	height = Schema.NonNegativeInteger,
	char_width = Schema.PositiveInteger,
	char_height = Schema.PositiveInteger,
	ascii_char_first = Schema.Optional(Schema.BoundedString(1, 1)),
	ascii_char_last = Schema.Optional(Schema.BoundedString(1, 1)),
}
function Text.Font.to_image(font)
	if expensive_debug_checks_enabled then
		assert(Text.Font.Schema(font))
	end

	local image = {
		filename = font.filename,
		file_type = font.file_type,
		u = font.u,
		v = font.v,
		width = font.width,
		height = font.height,
	}

	if expensive_debug_checks_enabled then
		assert(Image.Image.Schema(image))
	end

	return image
end

Text.Entity = {}
Text.Entity.Schema = Schema.AllOf(Entity.Entity.Schema, Schema.PartialObject{
	font_name = Schema.Optional(Schema.LabelString),
	text = Schema.Optional(Schema.String),
	text_max_width = Schema.Optional(Schema.PositiveInteger),
	text_max_height = Schema.Optional(Schema.PositiveInteger),
})

Text.WorldSys = World.Sys.new_metatable("text")
Text.WorldSys.tag = "text"
Text.WorldSys.font_default_name = "default"
Text.WorldSys.State = {}
Text.WorldSys.State.Schema = Schema.Object{
	fonts = Schema.Mapping(Schema.LabelString, Text.Font.Schema),
}
Text.WorldSys.State.defaults = {
	fonts = {
		[Text.WorldSys.font_default_name] = {
			filename = "./engine/data/font.png", file_type = "png", font_type = "ascii",
			u = 0, v = 0, width = 64, height = 96,
			char_width = 8, char_height = 8,
			ascii_char_first = ' ', ascii_char_last = '~',
		}
	},
}
Text.WorldSys.Schema = Schema.AllOf(World.Sys.Schema, Schema.PartialObject{
	state = Text.WorldSys.State.Schema,
	_client_world = Client.WorldSys.Schema,
	_entity_world = Entity.WorldSys.Schema,
	_image_world = Image.WorldSys.Schema,
	_ascii_fonts = Schema.Mapping(Schema.LabelString, Client.Wrappers.Schema("AsciiFont")),
})
local image_name_template = "font_%s"
function Text.WorldSys:draw(font_name, text, x, y, max_width, max_height, r, g, b, a, z)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Text.WorldSys.Schema(self))
		end
		assert(Schema.LabelString(font_name))
		assert(Schema.String(text))
		assert(Schema.Integer(x))
		assert(Schema.Integer(y))
		assert(Schema.Optional(Schema.PositiveInteger)(max_width))
		assert(Schema.Optional(Schema.PositiveInteger)(max_height))
		assert(Schema.Optional(Schema.BoundedInteger)(r, 0, 255))
		assert(Schema.Optional(Schema.BoundedInteger)(g, 0, 255))
		assert(Schema.Optional(Schema.BoundedInteger)(b, 0, 255))
		assert(Schema.Optional(Schema.BoundedInteger)(a, 0, 255))
		assert(Schema.Optional(Schema.Number)(z))
	end

	local font = self:font_find(font_name)

	local vertex_array = self._client_world:get_vertex_array()

	if font.font_type == Text.FontType.ascii then
		local ascii_font = self._ascii_fonts[font_name]
		ascii_font:add_text_to_vertex_array{
			vertex_array = vertex_array,
			str = text,
			x = x, y = y, max_w = max_width, max_h = max_height,
			color = {r or 0, g or 0, b or 0, a or 255},
			depth = z,
		}
	else
		error("Unknown font_type "..font.font_type)
	end
end
function Text.WorldSys:font_index(font_name, font)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Text.WorldSys.Schema(self))
			assert(Text.Font.Schema(font))
		end
		assert(Schema.LabelString(font_name))
	end

	font = font or self:font_find(font_name)

	self.state.fonts[font_name] = font

	local image_name = string.format(image_name_template, font_name)
	local image = Text.Font.to_image(font)
	self._image_world:set(image_name, image)

	local u1, v1 = font.u, font.v
	local image_allocator = self._image_world._allocator
	if image_allocator ~= nil then
		local image_bounds = image_allocator:get_image_bounds(image)
		u1 = image_bounds[1]
		v1 = image_bounds[2]
	end
	local font_bounds = {u1, v1, u1 + font.width, v1 + font.height}

	if font.font_type == Text.FontType.ascii then
		self._ascii_fonts[font_name] = Client.Wrappers.AsciiFont.new{
			u1 = font_bounds[1], v1 = font_bounds[2], u2 = font_bounds[3], v2 = font_bounds[4],
			char_w = font.char_width, char_h = font.char_height,
			char_first = font.ascii_char_first, char_last = font.ascii_char_last,
		}
	else
		error("Unknown font_type "..font.font_type)
	end
end
function Text.WorldSys:font_index_all()
	self._ascii_fonts = {}

	for font_name, font in pairs(self.state.fonts) do
		self:font_index(font_name, font)
	end
end
function Text.WorldSys:font_add(font_name, font)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Text.WorldSys.Schema(self))
			assert(Text.Font.Schema(font))
		end
		assert(Schema.LabelString(font_name))
	end

	self:font_index(font_name, font)
end
function Text.WorldSys:font_find(font_name)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Text.WorldSys.Schema(self))
		end
		assert(Schema.LabelString(font_name))
	end

	return self.state.fonts[font_name]
end
function Text.WorldSys:entity_set_text(entity_id, text, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Text.WorldSys.Schema(self))
			assert(Schema.Optional(Text.Entity.Schema)(entity))
		end
		assert(Schema.Optional(Schema.String)(text))
		assert(Schema.PositiveInteger(entity_id))
	end

	entity = entity or self._entity_world:find(entity_id)

	entity.text = text

	if entity.tags == nil or entity.tags["text"] ~= true then
		self._entity_world:tag(entity, {"text"})
	end
end
function Text.WorldSys:entity_unset(entity_id, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Text.WorldSys.Schema(self))
			assert(Schema.Optional(Text.Entity.Schema)(entity))
		end
		assert(Schema.PositiveInteger(entity_id))
	end

	entity = entity or self._entity_world:find(entity_id)

	entity.text = nil

	if entity.tags ~= nil and entity.tags["text"] == true then
		self._entity_world:untag(entity, {"text"})
	end
end
function Text.WorldSys:on_init()
	Container.set_defaults(self.state, Text.WorldSys.State.defaults)

	self._client_world = self.sim:require(Client.WorldSys)
	self._entity_world = self.sim:require(Entity.WorldSys)
	self._image_world = self.sim:require(Image.WorldSys)
	self._ascii_fonts = {}

	self:font_index_all()

	if expensive_debug_checks_enabled then
		assert(Text.WorldSys.Schema(self))
	end
end
function Text.WorldSys:on_draw()
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Text.WorldSys.Schema(self))
		end

		assert(self.sim.status == Sim.Status.started)
	end

	local text_entities = self._entity_world:get_all_tagged(self.tag)

	if expensive_debug_checks_enabled then
		assert(Schema.Array(Text.Entity.Schema)(text_entities))
	end

	for _, entity in ipairs(self._entity_world:get_all_tagged(self.tag)) do
		if entity.text ~= nil then
			self:draw(
				entity.font_name or self.font_default_name,
				entity.text,
				(entity.x or 0) + (entity.translate_x or 0), (entity.y or 0) + (entity.translate_y or 0),
				entity.max_width, entity.max_height,
				entity.r, entity.g, entity.b, entity.a,
				entity.z
			)
		end
	end
end

Text.tests = Testing.add_suite("engine.text", {
	run_game = function()
		local game = Game.Game.new({client = {visible = false}})
		-- local game = Game.Game.new()  -- to test with a non-headless client

		local WorldTestSys = World.Sys.new_metatable("world")
		function WorldTestSys:on_init()
			self._text_world = self.sim:require(Text.WorldSys)
		end
		function WorldTestSys:on_draw()
			self._text_world:draw("default", "wow", 4, 4)
		end

		local image_game = game:require(Image.GameSys)
		local world_game = game:require(World.GameSys)
		world_game:require_world_sys(Text.WorldSys)
		world_game:require_world_sys(WorldTestSys)

		local filename = "./examples/engine_test/data/sprites.png"
		image_game:load(filename)
		game:start()

		local world = world_game.world
		local entity_world = world:get(Entity.WorldSys)
		local text_world = world:get(Text.WorldSys)

		local entity_id = entity_world:add({
			tags = {text = true},
			text = "blargh",
			font_name = "test",
			x = 32, y = 32,
		})

		text_world:font_add("test", {
			filename = "./examples/engine_test/data/sprites.png",
			file_type = "png",
			font_type = "ascii",
			u = 0, v = 160, width = 64, height = 96,
			char_width = 8, char_height = 8,
			ascii_char_first = " ", ascii_char_last = "~"
		})

		for _ = 1, 10 do
			game:step()
			entity_world:set_pos(entity_id, 32 + game.step_id, 32 + math.floor(game.step_id / 2))
			text_world:entity_set_text(entity_id, "step "..game.step_id)
		end

		game:stop()
		game:finalize()
	end,
})

return Text
