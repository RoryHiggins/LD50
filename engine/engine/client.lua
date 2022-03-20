local schema = require("engine/core/schema")
local container = require("engine/core/container")

local wrappers = odClientWrapper  -- luacheck: globals odClientWrapper
local WrapperSchema = function(name)
	return schema.AllOf(schema.Userdata, schema.Check(function(x)
		if x._metatable_name == name then
			return true
		end
		return false, schema.error(
			"client.WrapperSchema(%s): metatable %s, expected %s", x, x._metatable_name, name)
	end))
end

local Settings = {}
Settings.schema = schema.Object{
	window = schema.Object{
		width = schema.NonNegativeInteger,
		height = schema.NonNegativeInteger,
	}
}
Settings.default = {
	window = {
		width = 640,
		height = 480,
	}
}

local Context = {}
Context.schema = schema.Object{
	window = WrapperSchema("Window"),
	texture_atlas = WrapperSchema("TextureAtlas"),
	renderer = WrapperSchema("Renderer"),
}
function Context.new(settings)
	settings = container.deep_copy(Settings.default, settings or {})
	assert(Settings.schema(settings))

	local context = {}
	context.window = wrappers.Window{width = 512, height = 512}
	context.texture_atlas = wrappers.TextureAtlas{window = context.window}
	context.renderer = wrappers.Renderer{window = context.window}
end

local client = {}
client.Context = Context
client.wrappers = wrappers
client.WrapperSchema = WrapperSchema

return client
