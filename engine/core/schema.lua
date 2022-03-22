local Testing = require("engine/core/testing")
local Logging = require("engine/core/logging")

local integer_min = -2^24
local integer_max = 2^24
local floor = math.floor

local Schema = {}
Schema.integer_min = integer_min
Schema.integer_max = integer_max
function Schema.error(format, ...)
	return string.format("Schema error:\n"..format, ...)
end
function Schema.Any()
	return true
end
function Schema.None(x)
	return false, Schema.error("Schema.None(%s)", x)
end
function Schema.Null(x)
	if x ~= nil then
		return false, Schema.error("Schema.Null(%s): not nil", x)
	end
	return true
end
function Schema.Boolean(x)
	if type(x) ~= "boolean" then
		return false, Schema.error("Schema.Boolean(%s): not boolean", x)
	end
	return true
end
function Schema.String(x)
	if type(x) ~= "string" then
		return false, Schema.error("Schema.String(%s): not a string", x)
	end
	return true
end
function Schema.Number(x)
	if type(x) ~= "number" then
		return false, Schema.error("Schema.Number(%s): not a number", x)
	end
	return true
end
function Schema.Function(x)
	if type(x) ~= "function" then
		return false, Schema.error("Schema.Function(%s): not a function", x)
	end
	return true
end
function Schema.Table(x)
	if type(x) ~= "table" then
		return false, Schema.error("Schema.Table(%s): not a table", x)
	end
	return true
end
function Schema.Userdata(x)
	if type(x) ~= "userdata" then
		return false, Schema.error("Schema.Userdata(%s): not userdata", x)
	end
	return true
end
function Schema.Array(condition, opt_length)
	assert(Schema.Function(condition))

	return function(x)
		if type(x) ~= "table" then
			return false, Schema.error("Schema.Array(%s): not a table", x)
		end

		if opt_length ~= nil and #x ~= opt_length then
			return false, Schema.error("Schema.Array(%s): unexpected length=%s, expected %s", x, #x, opt_length)
		end

		local count = 0
		for key, value in pairs(x) do
			if type(key) ~= "number" then
				return false, Schema.error("Schema.Array(%s): key=%s not a number", x, key)
			end
			local result, err = condition(value)
			if not result then
				return false, Schema.error("Schema.Array(%s): value=%s error=%s", x, value, err)
			end
			count = count + 1
		end

		if count > #x then
			return false, Schema.error("Schema.Array(%s): sparse array", x)
		end
		return true
	end
end
function Schema.Mapping(key_condition, value_condition)
	assert(Schema.Function(key_condition))
	assert(Schema.Function(value_condition))

	return function(x)
		if type(x) ~= "table" then
			return false, Schema.error("Schema.Mapping(%s): not a table", x)
		end
		if #x > 0 then
			return false, Schema.error("Schema.Mapping(%s): has positional elements", x)
		end

		for key, value in pairs(x) do
			local result, err = key_condition(key)
			if not result then
				return false, Schema.error(
					"Schema.Mapping(%s): invalid key=%s, value=%s error=%s", x, key, value, err)
			end

			result, err = value_condition(value)
			if not result then
				return false, Schema.error(
					"Schema.Mapping(%s): invalid value=%s, key=%s, error=%s", x, value, key, err)
			end
		end
		return true
	end
end
function Schema.Object(condition_map, additional_value_condition)
	assert(Schema.Mapping(Schema.String, Schema.Function)(condition_map))
	assert(Schema.Optional(Schema.Function)(additional_value_condition))
	additional_value_condition = additional_value_condition or Schema.None

	return function(x)
		if type(x) ~= "table" then
			return false, Schema.error("Schema.Object(%s): not a table", x)
		end
		if #x > 0 then
			return false, Schema.error("Schema.Object(%s): has positional elements", x)
		end

		-- first check all keys match conditions
		for key, value in pairs(x) do
			local condition = condition_map[key] or additional_value_condition
			if not type(key) == "string" then
				return false, Schema.error("Schema.Object(%s): non-string key=%s, value=%s", x, key, value)
			end

			local result, err = condition(value)
			if not result then
				return false, Schema.error("Schema.Object(%s): invalid value=%s, key=%s, error=%s", x, value, key, err)
			end
		end

		-- then check all conditions matched by key (keys could be missing)
		for key, condition in pairs(condition_map) do
			local value = x[key]
			local result, err = condition(value)
			if not result then
				return false, Schema.error(
					"Schema.Object(%s): invalid value=%s, key=%s, error=%s", x, value, key, err)
			end
		end
		return true
	end
end
function Schema.AnyOf(...)
	local conditions = {...}
	for _, condition in ipairs(conditions) do
		assert(Schema.Function(condition))
	end

	return function(x)
		local failures = {}
		for _, condition in ipairs(conditions) do
			local result, err = condition(x)
			if result then
				return true
			end
			failures[#failures + 1] = err
		end

		local failures_str = table.concat(failures, "\n")
		return false, Schema.error("Schema.AnyOf(%s): no match, match failures=\n%s", x, failures_str)
	end
end
function Schema.AllOf(...)
	local conditions = {...}
	for _, condition in ipairs(conditions) do
		assert(Schema.Function(condition))
	end

	return function(x)
		for _, condition in ipairs(conditions) do
			local result, err = condition(x)
			if not result then
				return false, Schema.error("Schema.AllOf(%s): no match, err=%s", x, err)
			end
		end
		return true
	end
end
function Schema.Enum(...)
	local values = {...}
	local values_str = table.concat(values, ", ")
	return function(x)
		for _, value in ipairs(values) do
			if x == value then
				return true
			end
		end
		return false, Schema.error("Schema.Enum(%s): no match, values={%s}", x, values_str)
	end
end
function Schema.Const(const)
	return function(x)
		if x ~= const then
			return false, Schema.error("Schema.Const(%s): no match, const=%s", x, const)
		end
		return true
	end
end
function Schema.Optional(condition)
	assert(Schema.Function(condition))

	return function(x)
		if x == nil then
			return true
		end
		local result, err = condition(x)
		if result then
			return true
		end
		return false, Schema.error("Schema.Optional(%s): no match, err=%s", x, err)
	end
end
function Schema.Check(condition)
	assert(Schema.Function(condition))

	return function(x)
		local result, err = condition(x)
		if not result then
			return false, Schema.error("Schema.Check(%s): no match, err=%s, check=%s", x, err, string.dump(condition))
		end
		return true
	end
end
function Schema.Integer(x)
	if type(x) == "number" and floor(x) == x and x >= integer_min and x <= integer_max then
		return true
	end
	return false, Schema.error("Schema.Integer(%s): no match", x)
end
function Schema.NormalizedNumber(x)
	if type(x) == "number" and x >= 0 and x <= 1 then
		return true
	end
	return false, Schema.error("Schema.NormalizedNumber(%s): no match", x)
end
function Schema.NonNegativeNumber(x)
	if type(x) == "number" and x >= 0 then
		return true
	end
	return false, Schema.error("Schema.NonNegativeNumber(%s): no match", x)
end
function Schema.NonNegativeInteger(x)
	if type(x) == "number" and floor(x) == x and x >= 0 and x <= integer_max then
		return true
	end
	return false, Schema.error("Schema.NonNegativeInteger(%s): no match", x)
end
function Schema.PositiveNumber(x)
	if type(x) == "number" and x > 0 then
		return true
	end
	return false, Schema.error("Schema.PositiveNumber(%s): no match", x)
end
function Schema.PositiveInteger(x)
	if type(x) == "number" and floor(x) == x and x > 0 and x <= integer_max then
		return true
	end
	return false, Schema.error("Schema.PositiveInteger(%s): no match", x)
end
function Schema.BoundedInteger(min, max)
	return function(x)
		if type(x) == "number" and floor(x) == x and x >= min and x <= max then
			return true
		end
		return false, Schema.error("Schema.BoundedInteger(%s): no match, min=%s, max=%s", x, min, max)
	end
end
local label_regex = "[a-z_][a-z0-9_]*"
function Schema.LabelString(x)
	if type(x) == "string" and string.match(x, label_regex) == x then
		return true
	end
	return false, Schema.error("Schema.LabelString(%s): no match, regex=%s", x, label_regex)
end
function Schema.NonEmptyString(x)
	if type(x) == "string" and #x > 0 then
		return true
	end
	return false, Schema.error("Schema.NonEmptyString(%s): no match", x)
end
function Schema.NonEmptyArray(condition, opt_length)
	assert(Schema.Function(condition))
	assert(Schema.Optional(Schema.PositiveInteger)(opt_length))
	local array_condition = Schema.Array(condition, opt_length)

	return function(x)
		local result, err = array_condition(x)
		if not result then
			return false, Schema.error("Schema.NonEmptyArray(%s): no match, err=%s", x, err)
		end
		if #x == 0 then
			return false, Schema.error("Schema.NonEmptyArray(%s): empty", x)
		end
		return true
	end
end
function Schema.PartialObject(condition_map)
	return Schema.Object(condition_map, Schema.Any)
end
local SerializableArray
local SerializableObject
function Schema.Serializable(x)
	local x_type = type(x)
	if x_type == "string" or x_type == "number" or x_type == "boolean" then
		return true
	end
	local array_result, array_err = SerializableArray(x)
	if array_result then
		return true
	end
	local object_result, object_err = SerializableObject(x)
	if object_result then
		return true
	end
	return false,  Schema.error("Schema.Serializable(%s): array_err=%s, object_err=%s", x, array_err, object_err)
end
SerializableArray = Schema.Array(Schema.Serializable)
SerializableObject = Schema.Mapping(Schema.String, Schema.Serializable)
Schema.SerializableArray = SerializableArray
Schema.SerializableObject = SerializableObject
Schema.AnyArray = Schema.Array(Schema.Any)
Schema.AnyObject = Schema.Object({}, Schema.Any)

Schema.tests = Testing.add_suite("core.Schema", {
	serialize_deserialize = function()
		local test_schema_values = {
			[Schema.Any] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[Schema.Boolean] = {true, false},
			[Schema.String] = {"", "ye", string.dump(Schema.Serializable)},
			[Schema.Number] = {-1, 0, 2, 0.312, 2e5, 0.000001, math.huge, math.pi},
			[Schema.Array(Schema.Any)] = {{}, {1, 2}, {1, 2, "3"}, {{},{"yes", y = 2}}},
			[Schema.Array(Schema.Number)] = {{}, {1, 2}, {1, 2}},
			[Schema.Mapping(Schema.String, Schema.Any)] =
				{{}, {a=2, b=3}, {a=2, b="ye"}, {a=2, b={{"ye", function() end}}}},
			[Schema.Mapping(Schema.String, Schema.String)] = {{}, {a="b", b="c"}, {a="b"}},
			[Schema.Object{a=Schema.String, b=Schema.Number}] = {{a="hello", b=2}},
			[Schema.Enum("hello", "World")] = {"hello", "World"},
			[Schema.Const("hello")] = {"hello"},
			[Schema.Optional(Schema.Number)] = {-1, 0, 2},  -- nil tested later, cant put in array
			[Schema.Check(function(x) return x % 2 == 0 end)] = {0, 2, 4, 6},
			[Schema.Integer] = {-1, 0, 2, 2e5, 2^24, -2^24},
			[Schema.NormalizedNumber] = {0, 1, 0.001, 0.999, 0.4},
			[Schema.NonNegativeNumber] = {0, 2, 0.312, 2e5, 0.000001, math.huge, math.pi},
			[Schema.PositiveNumber] = {0.002, 1, 2, 0.312, 2e5, 0.000001, math.huge, math.pi},
			[Schema.PositiveInteger] = {1, 2, 2e5},
			[Schema.BoundedInteger(2, 4)] = {2, 3, 4},
			[Schema.LabelString] = {"hello_world2", "a", "a2", "_az"},
			[Schema.NonEmptyString] = {"y", "hello"},
			[Schema.NonEmptyArray(Schema.Number)] = {{1}, {1, 2, 3, 4, 5}},
			[Schema.PartialObject{}] = {{}, {a=2}, {b=3}, {c=2, d={3, "4", {"5"}}}},
		}
		for value_schema, values in pairs(test_schema_values) do
			for _, value in ipairs(values) do
				local result, err = value_schema(value)
				if err then
					Logging.error(err)
				end
				assert(err == nil)
				assert(result == true)
			end
		end

		local test_schema_failure_values = {
			[Schema.None] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[Schema.Null] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[Schema.Boolean] = {-1, 0, 2, 0.312, "", "ye", function() end, {}, {1, 2}, {a=2, 3}},
			[Schema.String] = {-1, 0, 2, 0.312, true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[Schema.Number] = {"", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[Schema.Array(Schema.Any)] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {a = 2}},
			[Schema.Array(Schema.Number)] = {{{}}, {"ye"}, {1, 2, "3"}, {{},{"yes", y = 2}}},
			[Schema.Mapping(Schema.String, Schema.Any)] =
				{-1, 0, 2, 0.312, "", "ye", true, false, function() end, {1, 2}, {a=2, 3}},
			[Schema.Mapping(Schema.String, Schema.String)] =
				{-1, "", {1, 2}, {a=2, 3},  {a=2, b="ye"}, {a=2, b={{"ye", function() end}}}},
			[Schema.None] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[Schema.Enum("hello", "World")] = {"hello2", "world2", "hell", 2, true, 4, function() end},
			[Schema.Const("hello")] = {"hell", 2, {}},
			[Schema.Optional(Schema.Number)] = {"", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[Schema.Check(function(x) return x % 2 == 0 end)] = {1, 3, 5},
			[Schema.Integer] = {0.312, 0.000001, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[Schema.NormalizedNumber] = {-1, -2, 3, 400, -2^24, 2^24, -0.01, 1.0001},
			[Schema.NonNegativeNumber] = {-1, -2, -2^24, -math.huge, -math.pi},
			[Schema.NonNegativeInteger] = {0.312, 0.000001, math.huge, math.pi},
			[Schema.PositiveNumber] = {0, -1, -2, -2^24, -math.huge, -math.pi},
			[Schema.PositiveInteger] = {0, 0.312, 0.000001, math.huge, math.pi},
			[Schema.BoundedInteger(2, 4)] = {1, 5, 3.5 -1, 2000, 0.312, "", "ye", true, false, function() end, {}, {1, 2}},
			[Schema.LabelString] =
				{-1, 0, 0.312, "YA", "2a", "2", "yep nope", " ", "yE", true, false, function() end, {}, {1, 2}},
			[Schema.NonEmptyString] = {"", {}, {1}, {a = 1}, {nil}},
			[Schema.NonEmptyArray(Schema.Number)] = {"", "a", {}, {a = 1, 2}, {nil}, {"ye"}},
			[Schema.PartialObject{}] = {-1, "", {1, 2}, {a=2, 3}},
		}
		for value_schema, failure_values in pairs(test_schema_failure_values) do
			for _, failure_value in ipairs(failure_values) do
				local result, err = value_schema(failure_value)
				assert(result == false)
				assert(type(err) == "string")
			end
		end

		-- extra cases for nil and none which can't be captured as array elements:
		assert(Schema.Any())
		assert(Schema.Any(nil))
		assert(Schema.Null(nil))
		assert(Schema.Optional(Schema.Number)(nil))
		assert(not Schema.None())
		assert(not Schema.None(nil))
	end,
})

return Schema
