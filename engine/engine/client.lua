local debugging = require("engine/core/debugging")
local schema = require("engine/core/schema")
local container = require("engine/core/container")
local game = require("engine/engine/game")
local world = require("engine/engine/world")

local debug_checks_enabled = debugging.debug_checks_enabled

local wrappers = odClientWrapper  -- luacheck: globals odClientWrapper
function wrappers.Schema(name)
	return schema.AllOf(schema.Userdata, schema.Check(function(x)
		if x._metatable_name == name then
			return true
		end
		return false, schema.error(
			"client.wrappers.Schema(%s): metatable %s, expected %s", x, x._metatable_name, name)
	end))
end

local Context = {}
Context.__index = Context
Context.Settings = {}
Context.Settings.mouse_schema = schema.Object{
	x = schema.Integer,
	y = schema.Integer,
	left = schema.Boolean,
	middle = schema.Boolean,
	right = schema.Boolean,
}
Context.Settings.schema = schema.Object{
	window = schema.Object{
		caption = schema.String,
		width = schema.PositiveInteger,
		height = schema.PositiveInteger,
		fps_limit = schema.Optional(schema.PositiveInteger),
		vsync = schema.Boolean,
		visible = schema.Boolean,
	}
}
Context.Settings.defaults = {
	window = {
		caption = "",
		width = 512,
		height = 384,
		vsync = true,
		visible = true,
	}
}
Context.schema = schema.Object{
	settings = Context.Settings.schema,
	window = wrappers.Schema("Window"),
	texture_atlas = wrappers.Schema("TextureAtlas"),
	renderer = wrappers.Schema("Renderer"),
	vertex_array = wrappers.Schema("VertexArray"),
	render_state_ortho_2d = wrappers.Schema("RenderState"),
	render_state_passthrough = wrappers.Schema("RenderState"),
	mouse = Context.Settings.mouse_schema,
	step_count = schema.NonNegativeInteger,
}
function Context.new(settings)
	settings = container.object_set_defaults(container.deep_copy(settings or {}), Context.Settings.defaults)

	if debug_checks_enabled then
		assert(Context.Settings.schema(settings))
	end

	local context = {}
	context.settings = settings
	context.window = wrappers.Window.new(settings.window)
	context.texture_atlas = wrappers.TextureAtlas.new{window = context.window}
	context.renderer = wrappers.Renderer.new{window = context.window}
	context.vertex_array = wrappers.VertexArray.new{}
	context.render_state_ortho_2d = wrappers.RenderState.new_ortho_2d{target = context.window}
	context.render_state_passthrough = wrappers.RenderState.new{target = context.window}
	context.mouse = context.window:get_mouse_state()
	context.step_count = 0
	setmetatable(context, Context)

	if debug_checks_enabled then
		assert(Context.schema(context))
	end

	return context
end
function Context:step()
	if debug_checks_enabled then
		assert(Context.schema(self))
	end

	local running = self.window:step()
	if not running then
		return false
	end

	self.vertex_array:init{}
	self.render_state_ortho_2d:init_ortho_2d{target = self.window}
	self.render_state_passthrough:init{target = self.window}
	self.renderer:clear{color = {255, 255, 255, 255}}

	self.step_count = self.step_count + 1

	self.mouse = self.window:get_mouse_state()

	container.object_update(self.settings.window, self.window:get_settings())

	if debug_checks_enabled then
		assert(Context.schema(self))
	end

	return true
end

local RenderTarget = {}
RenderTarget.__index = RenderTarget
RenderTarget.Settings = {}
RenderTarget.Settings.schema = schema.Object{
	width = schema.PositiveInteger,
	height = schema.PositiveInteger,
}
RenderTarget.Settings.defaults = {
	width = 256,
	height = 192,
}
RenderTarget.schema = schema.Object{
	settings = RenderTarget.Settings.schema,
	context = Context.schema,
	render_texture = wrappers.Schema("RenderTexture"),
	vertex_array = wrappers.Schema("VertexArray"),
	render_state_ortho_2d = wrappers.Schema("RenderState"),
	render_state_passthrough = wrappers.Schema("RenderState"),
}
function RenderTarget.new(context, settings)
	settings = container.object_set_defaults(container.deep_copy(settings or {}), RenderTarget.Settings.defaults)

	if debug_checks_enabled then
		assert(Context.schema(context))
		assert(RenderTarget.Settings.schema(settings))
	end

	local render_target = {}
	render_target.settings = settings
	render_target.context = context
	render_target.render_texture = wrappers.RenderTexture.new{
		window = context.window, width = settings.width, height = settings.height}
	render_target.vertex_array = wrappers.VertexArray.new{}
	render_target.render_state_ortho_2d = wrappers.RenderState.new_ortho_2d{target = render_target.render_texture}
	render_target.render_state_passthrough = wrappers.RenderState.new{target = render_target.render_texture}
	setmetatable(render_target, RenderTarget)

	if debug_checks_enabled then
		assert(RenderTarget.schema(render_target))
	end

	return render_target
end
function RenderTarget:step()
	if debug_checks_enabled then
		assert(RenderTarget.schema(self))
	end

	self.vertex_array:init{}
	self.render_state_ortho_2d:init_ortho_2d{target = self.render_texture}
	self.render_state_passthrough:init{target = self.render_texture}
	self.context.renderer:clear{target = self.render_texture, color = {255, 255, 255, 255}}
end

local WorldSys = world.Sys.new_metatable("client")
function WorldSys:on_start()
	self.render_target = RenderTarget.new(self._game_client.context, self.settings)
end
function WorldSys:on_step()
	self.render_target:step()
end

local GameSys = game.Sys.new_metatable("client")
function GameSys:on_init()
	if debug_checks_enabled then
		assert(GameSys.schema(self))
	end

	self.settings = container.object_set_defaults(container.deep_copy(self.settings or {}), Context.Settings.defaults)
	self.context = Context.new(self.settings)

	if debug_checks_enabled then
		assert(Context.schema(self.context))
		assert(Context.Settings.schema(self.settings))
	end

	self._world_game = self.sim:require(world.GameSys)
end
function GameSys:on_step()
	if not self.context:step() then
		self.sim.finalize()
	end

	container.object_update(self.settings, self.context.settings)
end
function GameSys:on_world_init()
	local world_client = self._world_game.world:require(WorldSys)
	world_client._game_client = self
end

local client = {}
client.wrappers = wrappers
client.Context = Context
client.RenderTarget = RenderTarget
client.WorldSys = WorldSys
client.GameSys = GameSys

return client
