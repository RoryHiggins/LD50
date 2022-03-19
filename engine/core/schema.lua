local testing = require("engine/core/testing")

local integer_min = -2^24
local integer_max = 2^24
local floor = math.floor

local schema = {}
schema.integer_min = integer_min
schema.integer_max = integer_max
function schema.Any()
	return true
end
function schema.None()
	return false
end
function schema.Null(x)
	return x == nil
end
function schema.Boolean(x)
	return type(x) == "boolean"
end
function schema.String(x)
	return type(x) == "string"
end
function schema.Function(x)
	return type(x) == "function"
end
function schema.Number(x)
	return type(x) == "number"
end
function schema.Integer(x)
	return type(x) == "number" and floor(x) == x and x >= integer_min and x <= integer_max
end
function schema.Normalized(x)
	return type(x) == "number" and x >= 0 and x <= 1
end
function schema.NonNegative(x)
	return type(x) == "number" and x >= 0
end
function schema.NonEmpty(x)
	local type_x = type(x)
	return (type_x == "string" or type_x == "table") and #x > 0
end
function schema.Array(condition)
	assert(type(condition) == "function")

	return function(x)
		if type(x) ~= "table" then
			return false
		end

		local count = 0
		for key, value in pairs(x) do
			if type(key) ~= "number" or not condition(value) then
				return false
			end
			count = count + 1
		end

		if count > #x then
			return false  -- sparse array
		end
		return true
	end
end
function schema.Mapping(key_condition, value_condition)
	assert(type(key_condition) == "function")
	assert(type(value_condition) == "function")

	return function(x)
		if type(x) ~= "table" or #x > 0 then
			return false
		end

		for key, value in pairs(x) do
			if not key_condition(key) or not value_condition(value) then
				return false
			end
		end
		return true
	end
end
local condition_map_schema = schema.Mapping(schema.String, schema.Function)
function schema.Object(condition_map, additional_value_condition)
	assert(type(condition_map) == "table")
	assert(condition_map_schema(condition_map))
	assert(additional_value_condition == nil or type(additional_value_condition) == "function")
	additional_value_condition = additional_value_condition or schema.None

	return function(x)
		if type(x) ~= "table" or #x > 0 then
			return false
		end

		-- first check all keys match conditions
		for key, value in pairs(x) do
			local condition = condition_map[key] or additional_value_condition
			if not type(key) == "string" or not condition(value) then
				return false
			end
		end

		-- then check all conditions matched by key (keys could be missing)
		for key, condition in pairs(condition_map) do
			local value = x[key]
			if not condition(value) then
				return false
			end
		end
		return true
	end
end
function schema.AnyOf(...)
	local conditions = {...}
	return function(x)
		for _, condition in ipairs(conditions) do
			if condition(x) then
				return true
			end
		end
		return false
	end
end
function schema.AllOf(...)
	local conditions = {...}
	return function(x)
		for _, condition in ipairs(conditions) do
			if not condition(x) then
				return false
			end
		end
		return true
	end
end
function schema.Enum(...)
	local values = {...}
	return function(x)
		for _, value in ipairs(values) do
			if x == value then
				return true
			end
		end
		return false
	end
end
function schema.Const(const)
	return function(x)
		return x == const
	end
end
function schema.Optional(condition)
	return function(x)
		return x == nil or condition(x)
	end
end
function schema.Check(fn)
	return fn
end
local SerializableArray
local SerializableObject
function schema.Serializable(x)
	local x_type = type(x)
	return (
		x_type == "string"
		or x_type == "number"
		or x_type == "boolean"
		or SerializableArray(x)
		or SerializableObject(x)
	)
end
SerializableArray = schema.Array(schema.Serializable)
SerializableObject = schema.Mapping(schema.String, schema.Serializable)

schema.tests = testing.add_suite("core.schema", {
	serialize_deserialize = function()
		local test_schema_values = {
			[schema.Any] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Boolean] = {true, false},
			[schema.String] = {"", "ye", string.dump(schema.Serializable)},
			[schema.Number] = {-1, 0, 2, 0.312, 2e5, 0.000001, math.huge, math.pi},
			[schema.Integer] = {-1, 0, 2, 2e5, 2^24, -2^24},
			[schema.Normalized] = {0, 1, 0.001, 0.999, 0.4},
			[schema.NonNegative] = {0, 2, 0.312, 2e5, 0.000001, math.huge, math.pi},
			[schema.NonEmpty] = {"y", {1}, {1, 2, 3, 4, 5}},
			[schema.Array(schema.Any)] = {{}, {1, 2}, {1, 2, "3"}, {{},{"yes", y = 2}}},
			[schema.Array(schema.Number)] = {{}, {1, 2}, {1, 2}},
			[schema.Mapping(schema.String, schema.Any)] =
				{{}, {a=2, b=3}, {a=2, b="ye"}, {a=2, b={{"ye", function() end}}}},
			[schema.Mapping(schema.String, schema.String)] = {{}, {a="b", b="c"}, {a="b"}},
			[schema.Object({a=schema.String, b=schema.Number})] = {{a="hello", b=2}},
			[schema.Object({}, schema.Any)] = {{}, {a=2}, {b=3}, {c=2, d={3, "4", {"5"}}}},
			[schema.Enum("hello", "world")] = {"hello", "world"},
			[schema.Const("hello")] = {"hello"},
			[schema.Optional(schema.Number)] = {-1, 0, 2},  -- nil tested later, cant put in array
			[schema.Check(function(x) return x % 2 == 0 end)] = {0, 2, 4, 6},
		}
		for value_schema, values in pairs(test_schema_values) do
			for _, value in ipairs(values) do
				assert(value_schema(value))
			end
		end

		local test_schema_failure_values = {
			[schema.None] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Null] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Boolean] = {-1, 0, 2, 0.312, "", "ye", function() end, {}, {1, 2}, {a=2, 3}},
			[schema.String] = {-1, 0, 2, 0.312, true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Number] = {"", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Integer] = {0.312, 0.000001, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Normalized] = {-1, -2, 3, 400, -2^24, 2^24, -0.01, 1.0001},
			[schema.NonNegative] = {-1, -2, -2^24, -math.huge, -math.pi},
			[schema.NonEmpty] = {"", {}, {a = 1}, {nil}},
			[schema.Array(schema.Any)] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {a = 2}},
			[schema.Array(schema.Number)] = {{{}}, {"ye"}, {1, 2, "3"}, {{},{"yes", y = 2}}},
			[schema.Mapping(schema.String, schema.Any)] =
				{-1, 0, 2, 0.312, "", "ye", true, false, function() end, {1, 2}, {a=2, 3}},
			[schema.Mapping(schema.String, schema.String)] =
				{-1, "", {1, 2}, {a=2, 3},  {a=2, b="ye"}, {a=2, b={{"ye", function() end}}}},
			[schema.Object({}, schema.Any)] = {-1, "", {1, 2}, {a=2, 3}},
			[schema.None] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Enum("hello", "world")] = {"hello2", "world2", "hell", 2, true, 4, function() end},
			[schema.Const("hello")] = {"hell", 2, {}},
			[schema.Optional(schema.Number)] = {"", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Check(function(x) return x % 2 == 0 end)] = {1, 3, 5},
		}
		for value_schema, failure_values in pairs(test_schema_failure_values) do
			for _, failure_value in ipairs(failure_values) do
				testing.assert_fails(function()
					assert(value_schema(failure_value))
				end)
			end
		end

		-- extra cases for nil and none which can't be captured as array elements:
		assert(schema.Any())
		assert(schema.Any(nil))
		assert(schema.Null(nil))
		assert(schema.Optional(schema.Number)(nil))
		testing.assert_fails(function()
			assert(schema.None())
		end)
		testing.assert_fails(function()
			assert(schema.None(nil))
		end)
	end,
})
testing.run_all()

return schema
