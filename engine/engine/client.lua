local Debugging = require("engine/core/debugging")
local Schema = require("engine/core/Schema")
local Container = require("engine/core/container")
local Testing = require("engine/core/testing")
local Game = require("engine/engine/game")
local World = require("engine/engine/world")

local debug_checks_enabled = Debugging.debug_checks_enabled

local Client = {}

Client.wrappers = odClientWrapper  -- luacheck: globals odClientWrapper
function Client.wrappers.Schema(name)
	return Schema.AllOf(Schema.Userdata, Schema.Check(function(x)
		if x._metatable_name == name then
			return true
		end
		return false, Schema.error(
			"Client.wrappers.Schema(%s): metatable %s, expected %s", x, x._metatable_name, name)
	end))
end

Client.Context = {}
Client.Context.__index = Client.Context
Client.Context.Settings = {}
Client.Context.Settings.Schema = Schema.Object{
	window = Schema.Optional(Schema.Object{
		caption = Schema.Optional(Schema.String),
		width = Schema.Optional(Schema.PositiveInteger),
		height = Schema.Optional(Schema.PositiveInteger),
		vsync = Schema.Optional(Schema.Boolean),
		visible = Schema.Optional(Schema.Boolean),
	})
}
Client.Context.Settings.defaults = {
	window = {
		caption = "",
		width = 512,
		height = 384,
		vsync = true,
		visible = true,
	}
}
Client.Context.Schema = Schema.Object{
	settings = Client.Context.Settings.Schema,
	window = Client.wrappers.Schema("Window"),
	texture_atlas = Client.wrappers.Schema("TextureAtlas"),
	renderer = Client.wrappers.Schema("Renderer"),
	vertex_array = Client.wrappers.Schema("VertexArray"),
	render_state_ortho_2d = Client.wrappers.Schema("RenderState"),
	render_state_passthrough = Client.wrappers.Schema("RenderState"),
	mouse = Schema.Object{
		x = Schema.Integer,
		y = Schema.Integer,
		left = Schema.Boolean,
		middle = Schema.Boolean,
		right = Schema.Boolean,
	},
	step_count = Schema.NonNegativeInteger,
}
function Client.Context.new(settings)
	settings = Container.set_defaults({}, settings or {}, Client.Context.Settings.defaults)

	if debug_checks_enabled then
		assert(Client.Context.Settings.Schema(settings))
	end

	local context = {}
	context.settings = settings
	context.window = Client.wrappers.Window.new(settings.window)
	context.texture_atlas = Client.wrappers.TextureAtlas.new{window = context.window}
	context.renderer = Client.wrappers.Renderer.new{window = context.window}
	context.vertex_array = Client.wrappers.VertexArray.new{}
	context.render_state_ortho_2d = Client.wrappers.RenderState.new_ortho_2d{target = context.window}
	context.render_state_passthrough = Client.wrappers.RenderState.new{target = context.window}
	context.mouse = context.window:get_mouse_state()
	context.step_count = 0
	setmetatable(context, Client.Context)

	if debug_checks_enabled then
		assert(Client.Context.Schema(context))
	end

	return context
end
function Client.Context:step()
	if debug_checks_enabled then
		assert(Client.Context.Schema(self))
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

	Container.update(self.settings.window, self.window:get_settings())

	if debug_checks_enabled then
		assert(Client.Context.Schema(self))
	end

	return true
end

Client.RenderTarget = {}
Client.RenderTarget.__index = Client.RenderTarget
Client.RenderTarget.Settings = {}
Client.RenderTarget.Settings.Schema = Schema.Object{
	width = Schema.PositiveInteger,
	height = Schema.PositiveInteger,
}
Client.RenderTarget.Settings.defaults = {
	width = 256,
	height = 192,
}
Client.RenderTarget.Schema = Schema.Object{
	settings = Client.RenderTarget.Settings.Schema,
	context = Client.Context.Schema,
	render_texture = Client.wrappers.Schema("RenderTexture"),
	vertex_array = Client.wrappers.Schema("VertexArray"),
	render_state_ortho_2d = Client.wrappers.Schema("RenderState"),
	render_state_passthrough = Client.wrappers.Schema("RenderState"),
}
function Client.RenderTarget.new(context, settings)
	settings = Container.update({}, Client.RenderTarget.Settings.defaults, settings or {})

	if debug_checks_enabled then
		assert(Client.Context.Schema(context))
		assert(Client.RenderTarget.Settings.Schema(settings))
	end

	local render_target = {}
	render_target.settings = settings
	render_target.context = context
	render_target.render_texture = Client.wrappers.RenderTexture.new{
		window = context.window, width = settings.width, height = settings.height}
	render_target.vertex_array = Client.wrappers.VertexArray.new{}
	render_target.render_state_ortho_2d = Client.wrappers.RenderState.new_ortho_2d{target = render_target.render_texture}
	render_target.render_state_passthrough = Client.wrappers.RenderState.new{target = render_target.render_texture}
	setmetatable(render_target, Client.RenderTarget)

	if debug_checks_enabled then
		assert(Client.RenderTarget.Schema(render_target))
	end

	return render_target
end
function Client.RenderTarget:step()
	if debug_checks_enabled then
		assert(Client.RenderTarget.Schema(self))
	end

	self.vertex_array:init{}
	self.render_state_ortho_2d:init_ortho_2d{target = self.render_texture}
	self.render_state_passthrough:init{target = self.render_texture}
	self.context.renderer:clear{target = self.render_texture, color = {255, 255, 255, 255}}
end

Client.WorldSys = World.Sys.new_metatable("client")
Client.WorldSys.Settings = {}
Client.WorldSys.Settings.Schema = Schema.Object{
	render_target = Client.RenderTarget.Settings.Schema,
}
Client.WorldSys.Settings.defaults = {
	render_target = Client.RenderTarget.Settings.defaults,
}
function Client.WorldSys:on_start()
	if self._game_client.state.headless then
		return
	end

	self.render_target = Client.RenderTarget.new(self._game_client.context, self.settings)
end
function Client.WorldSys:on_step()
	if self._game_client.state.headless then
		return
	end

	self.render_target:step()
end

Client.GameSys = Game.Sys.new_metatable("client")
Client.GameSys.Settings = {}
Client.GameSys.Settings.Schema = Schema.Object{
	headless = Schema.Optional(Schema.Boolean),
	context = Schema.Optional(Client.Context.Settings.Schema),
}
Client.GameSys.Settings.defaults = {
	headless = false,
	context = Client.Context.Settings.defaults,
}
Client.GameSys.State = Client.GameSys.Settings
function Client.GameSys:on_init()
	Container.set_defaults(self.settings, Client.GameSys.Settings.defaults)
	Container.set_defaults(self.state, Client.GameSys.State.defaults)

	if not self.settings.headless then
		self.context = Client.Context.new(self.settings.context)
	end

	Container.update(self.state, self.settings)

	if debug_checks_enabled then
		assert(Client.GameSys.Settings.Schema(self.settings))
		assert(Client.GameSys.State.Schema(self.state))
		assert(Schema.Optional(Client.Context.Schema)(self.context))
	end
end
function Client.GameSys:on_step()
	if self.state.headless then
		return
	end

	if not self.context:step() then
		self.sim.enqueue_finalize()
	end

	Container.update(self.state.context, self.context.settings)
end
function Client.GameSys:on_world_init()
	self.sim:get(World.GameSys).world:require(Client.WorldSys)._game_client = self
end

Client.tests = Testing.add_suite("engine.client", {
	run_headless = function()
		local game_sim = Game.Game.new({}, {client = {headless = true}})
		game_sim:require(Client.GameSys)
		game_sim:start()
		game_sim:step()
		game_sim:finalize()
	end
})

return Client
