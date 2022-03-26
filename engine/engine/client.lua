local Debugging = require("engine/core/debugging")
local Schema = require("engine/core/Schema")
local Container = require("engine/core/container")
local Testing = require("engine/core/testing")
local Game = require("engine/engine/game")
local World = require("engine/engine/world")
local Camera = require("engine/engine/camera")

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
	window = Schema.Object{
		width = Schema.PositiveInteger,
		height = Schema.PositiveInteger,
		caption = Schema.Optional(Schema.String),
		vsync = Schema.Optional(Schema.Boolean),
		visible = Schema.Optional(Schema.Boolean),
	}
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
Client.WorldSys.State = {}
Client.WorldSys.State.Schema = Client.WorldSys.Settings.Schema
Client.WorldSys.State.defaults = Client.WorldSys.Settings.defaults
function Client.WorldSys:draw()
	local world_vertex_array = self._vertex_array
	world_vertex_array:init{}

	self.sim:broadcast("on_draw")

	if self._context == nil then
		return
	end

	self._render_target:step()

	for _, camera in ipairs(self._camera_sys:all_depth_ordered()) do
		local camera_render_state = self._render_target.render_state_ortho_2d:copy()
		if camera.transform ~= nil then
			camera_render_state:transform_view(camera.transform)
		end
		if camera.viewport ~= nil then
			camera_render_state:set_viewport_ortho_2d(camera.viewport)
		end

		self._context.renderer:clear{
			target = self._render_target.render_texture,
			color = {255, 255, 255, 255},
		}
		self._context.renderer:draw_vertex_array{
			render_state = camera_render_state,
			src = self._context.texture_atlas,
			target = self._render_target.render_texture,
			vertex_array = self._vertex_array
		}
	end

	self._context.renderer:draw_texture{
		render_state = self._context.render_state_passthrough,
		src = self._render_target.render_texture,
	}
end
function Client.WorldSys:get_vertex_array()
	return self._vertex_array
end
function Client.WorldSys:get_size()
	return self.state.render_target.width, self.state.render_target.height
end
function Client.WorldSys:_get_mouse_pos()
	if self._context == nil then
		return 0, 0
	end

	local mouse_x, mouse_y = self._context.mouse.x, self._context.mouse.y
	local width, height = self:get_size()
	local game_width, game_height = self._context.settings.window.width, self._context.settings.window.height
	return ((mouse_x / game_width) * width), ((mouse_y / game_height) * height)
end
function Client.WorldSys:set_size(width, height)
	if debug_checks_enabled then
		assert(Schema.PositiveInteger(width))
		assert(Schema.PositiveInteger(height))
	end

	self.state.render_target.width = width
	self.state.render_target.height = height
	if self._context ~= nil then
		self._render_target = Client.RenderTarget.new(self._context, self.state.render_target)
	end
end
function Client.WorldSys:on_init()
	Container.set_defaults(self.settings, Client.WorldSys.Settings.defaults)
	Container.set_defaults(self.state, Client.WorldSys.State.defaults)

	self._camera_sys = self.sim:require(Camera.WorldSys)
	self._vertex_array = Client.wrappers.VertexArray.new{}
	self._context = self.sim._game._context

	Container.update(self.state, self.settings)
	if self._context ~= nil then
		self._render_target = Client.RenderTarget.new(self._context, self.state.render_target)
	end

	if debug_checks_enabled then
		assert(Client.WorldSys.Settings.Schema(self.settings))
		assert(Client.WorldSys.State.Schema(self.state))
		assert(Schema.Optional(Client.Context.Schema)(self._context))
	end
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

function Client.GameSys:get_size()
	local context_state = self.state.context
	if context_state == nil then
		return 0, 0
	end

	return context_state.window.width, context_state.window.height
end
function Client.GameSys:get_mouse_pos()
	if self.context == nil then
		return 0, 0
	end

	return self.context.mouse.x, self.context.mouse.y
end
function Client.GameSys:get_vertex_array()
	return self._vertex_array
end
function Client.GameSys:draw()
	self._vertex_array:init{}

	if self.context ~= nil then
		self.context.renderer:clear{
			color = {255, 255, 255, 255},
		}
	end

	local world = self._world_sys.world
	if world ~= nil then
		world:get(Client.WorldSys):draw()
	end

	self.sim:broadcast("on_draw")

	if self.context == nil then
		return
	end

	self.context.renderer:draw_vertex_array{
		render_state = self.context.render_state_ortho_2d,
		src = self.context.texture_atlas,
		vertex_array = self._vertex_array
	}
end
function Client.GameSys:on_init()
	Container.set_defaults(self.settings, Client.GameSys.Settings.defaults)
	Container.set_defaults(self.state, Client.GameSys.State.defaults)

	self._vertex_array = Client.wrappers.VertexArray.new{}
	self._world_sys = self.sim:require(World.GameSys)
	self._world_sys:require_world_sys(Client.WorldSys)

	if self.settings.headless ~= true then
		self.context = Client.Context.new(self.settings.context)
		self.sim._context = self.context
	end

	Container.update(self.state, self.settings)

	if debug_checks_enabled then
		assert(Client.GameSys.Settings.Schema(self.settings))
		assert(Client.GameSys.State.Schema(self.state))
		assert(Schema.Optional(Client.Context.Schema)(self.context))
	end
end
function Client.GameSys:on_step()
	if self.context == nil then
		return
	end

	if not self.context:step() then
		self.sim:enqueue_finalize()
		return
	end

	self:draw()

	Container.update(self.state.context, self.context.settings)
end

Client.tests = Testing.add_suite("engine.client", {
	run_headless = function()
		local game = Game.Game.new({}, {client = {headless = true}})
		local client_game = game:require(Client.GameSys)
		local world_sys = game:require(World.GameSys)

		game:start()
		local width, height = client_game:get_size()
		local mouse_x, mouse_y = client_game:get_mouse_pos()
		game:step()
		client_game:draw()
		assert(Schema.PositiveInteger(width))
		assert(Schema.PositiveInteger(height))
		assert(Schema.NonNegativeInteger(mouse_x))
		assert(Schema.NonNegativeInteger(mouse_y))

		local world = world_sys.world
		local client_world = world:get(Client.WorldSys)
		assert(type(client_world:get_vertex_array()) == "userdata")
		width, height = client_world:get_size()
		mouse_x, mouse_y = client_world:_get_mouse_pos()
		assert(Schema.PositiveInteger(width))
		assert(Schema.PositiveInteger(height))
		assert(Schema.NonNegativeInteger(mouse_x))
		assert(Schema.NonNegativeInteger(mouse_y))

		client_world:set_size(32, 32)
		width, height = client_world:get_size()
		world:step()
		client_world:draw()
		assert(width == 32)
		assert(height == 32)

		world:finalize()
		game:finalize()
	end
})

return Client
