local Debugging = require("engine/core/debugging")
local Testing = require("engine/core/testing")
local Schema = require("engine/core/schema")
local Container = require("engine/core/container")
local World = require("engine/engine/world")
local Game = require("engine/engine/game")
local Entity = require("engine/engine/entity")
local Client = require("engine/engine/client")
local Image = require("engine/engine/image")
local Text = require("engine/engine/text")

local debug_checks_enabled = Debugging.debug_checks_enabled
local expensive_debug_checks_enabled = Debugging.expensive_debug_checks_enabled

local Template = {}

Template.Template = {}
Template.Template.Schema = Entity.Entity.Schema

Template.WorldSys = World.Sys.new_metatable("template")
Template.WorldSys.State = {}
Template.WorldSys.State.Schema = Schema.Object{
	templates = Schema.Mapping(Schema.LabelString, Template.Template.Schema)
}
Template.WorldSys.State.defaults = {
	templates = {},
}
Template.WorldSys.Schema = Schema.AllOf(World.Sys.Schema, Schema.PartialObject{
	state = Template.WorldSys.State.Schema,
	_entity = Entity.WorldSys.Schema,
})
function Template.WorldSys:set(template_name, template)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Template.WorldSys.Schema(self))
			assert(Schema.Optional(Template.Template.Schema)(template))
		end
		assert(Schema.LabelString(template_name))
	end

	self.state.templates[template_name] = template
end
function Template.WorldSys:update(template_name, template)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Template.WorldSys.Schema(self))
			assert(Schema.Optional(Template.Template.Schema)(template))
		end
		assert(Schema.LabelString(template_name))
	end

	self.state.templates[template_name] = self.state.templates[template_name] or {}
	Container.update(self.state.templates[template_name], template)
end
function Template.WorldSys:set_defaults(template_name, template)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Template.WorldSys.Schema(self))
			assert(Schema.Optional(Template.Template.Schema)(template))
		end
		assert(Schema.LabelString(template_name))
	end

	self.state.templates[template_name] = self.state.templates[template_name] or {}
	Container.set_defaults(self.state.templates[template_name], template)
end
function Template.WorldSys:find(template_name)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Template.WorldSys.Schema(self))
		end
		assert(Schema.LabelString(template_name))
	end

	return self.state.templates[template_name]
end
function Template.WorldSys:instantiate(template_name, entity)
	if debug_checks_enabled then
		if expensive_debug_checks_enabled then
			assert(Template.WorldSys.Schema(self))
		end
		assert(Schema.LabelString(template_name))
	end

	entity = entity or {}

	local template = self.state.templates[template_name]
	Container.set_defaults(entity, template)
	return self._entity:add(entity)
end
function Template.WorldSys:on_init()
	Container.set_defaults(self.state, Template.WorldSys.State.defaults)

	self._entity = self.sim:require(Entity.WorldSys)

	if expensive_debug_checks_enabled then
		assert(Template.WorldSys.Schema(self))
	end
end

Template.GameSys = Game.Sys.new_metatable("template")
Template.GameSys.WorldSys = Template.WorldSys

Template.tests = Testing.add_suite("engine.template", {
	run_game = function()
		local game = Game.Game.new({client = {visible = false}})
		-- local game = Game.Game.new()  -- to test with a non-headless client

		local WorldTestSys = World.Sys.new_metatable("world")
		function WorldTestSys:on_init()
			self._template = self.sim:require(Template.WorldSys)
			self._template:set("label", {
				text = "hello",
				tags = {text = true},
			})
		end
		function WorldTestSys:on_start()
			self._template:instantiate("label", {x = 8, y = 8})

			self._template:instantiate("label", {x = 8, y = 32})
		end

		game:require(Client.GameSys)
		game:require(Image.GameSys)
		game:require(Entity.GameSys)
		game:require(Text.GameSys)
		game:require(Template.GameSys)
		local world_game = game:require(World.GameSys)
		world_game:require_world_sys(WorldTestSys)
		game:start()

		for _ = 1, 3 do
			game:step()
		end

		game:stop()
		game:finalize()
	end,
})

return Template
