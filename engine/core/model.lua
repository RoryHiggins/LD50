local Schema = require("engine/core/schema")
local Container = require("engine/core/container")

local Model = {}
function Model.AnyEnum(...)
	local Enum = {}
	Enum.enum_values = {...}
	Enum.enum_value_set = {}

	assert(#Enum.enum_values > 0)
	for _, value in ipairs(Enum.enum_values) do
		assert(Schema.String(value))
		Enum.enum_value_set[value] = true
	end
	Enum.Schema = Schema.Enum(...)

	return Enum
end

function Model.Enum(...)
	local Enum = Model.AnyEnum(...)

	for _, value in ipairs(Enum.enum_values) do
		assert(Schema.LabelString(value))
		assert(not Container.array_try_find({"enum_values", "enum_value_set", "Schema", value}))
		Enum[value] = value
	end

	return Enum
end

return Model
