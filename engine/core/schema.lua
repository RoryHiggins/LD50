local testing = require("engine/core/testing")
local logging = require("engine/core/logging")

local integer_min = -2^24
local integer_max = 2^24
local floor = math.floor

local schema = {}
schema.integer_min = integer_min
schema.integer_max = integer_max
function schema.error(format, ...)
	return string.format("Schema error:\n%s\n"..format, debug.traceback(), ...)
end
function schema.Any()
	return true
end
function schema.None(x)
	return false, schema.error("schema.None(%s)", x)
end
function schema.Null(x)
	if x ~= nil then
		return false, schema.error("schema.Null(%s): not nil", x)
	end
	return true
end
function schema.Boolean(x)
	if type(x) ~= "boolean" then
		return false, schema.error("schema.Boolean(%s): not boolean", x)
	end
	return true
end
function schema.String(x)
	if type(x) ~= "string" then
		return false, schema.error("schema.String(%s): not a string", x)
	end
	return true
end
function schema.Number(x)
	if type(x) ~= "number" then
		return false, schema.error("schema.Number(%s): not a number", x)
	end
	return true
end
function schema.Function(x)
	if type(x) ~= "function" then
		return false, schema.error("schema.Function(%s): not a function", x)
	end
	return true
end
function schema.Table(x)
	if type(x) ~= "table" then
		return false, schema.error("schema.Table(%s): not a table", x)
	end
	return true
end
function schema.Userdata(x)
	if type(x) ~= "userdata" then
		return false, schema.error("schema.Userdata(%s): not userdata", x)
	end
	return true
end
function schema.Array(condition, opt_length)
	assert(schema.Function(condition))

	return function(x)
		if type(x) ~= "table" then
			return false, schema.error("schema.Array(%s): not a table", x)
		end

		if opt_length ~= nil and #x ~= opt_length then
			return false, schema.error("schema.Array(%s): unexpected length=%s, expected %s", x, #x, opt_length)
		end

		local count = 0
		for key, value in pairs(x) do
			if type(key) ~= "number" then
				return false, schema.error("schema.Array(%s): key=%s not a number", x, key)
			end
			local result, err = condition(value)
			if not result then
				return false, schema.error("schema.Array(%s): value=%s error=%s", x, value, err)
			end
			count = count + 1
		end

		if count > #x then
			return false, schema.error("schema.Array(%s): sparse array", x)
		end
		return true
	end
end
function schema.Mapping(key_condition, value_condition)
	assert(schema.Function(key_condition))
	assert(schema.Function(value_condition))

	return function(x)
		if type(x) ~= "table" then
			return false, schema.error("schema.Mapping(%s): not a table", x)
		end
		if #x > 0 then
			return false, schema.error("schema.Mapping(%s): has positional elements", x)
		end

		for key, value in pairs(x) do
			local result, err = key_condition(key)
			if not result then
				return false, schema.error(
					"schema.Mapping(%s): invalid key=%s, value=%s error=%s", x, key, value, err)
			end

			result, err = value_condition(value)
			if not result then
				return false, schema.error(
					"schema.Mapping(%s): invalid value=%s, key=%s, error=%s", x, value, key, err)
			end
		end
		return true
	end
end
function schema.Object(condition_map, additional_value_condition)
	assert(schema.Mapping(schema.String, schema.Function)(condition_map))
	assert(schema.Optional(schema.Function)(additional_value_condition))
	additional_value_condition = additional_value_condition or schema.None

	return function(x)
		if type(x) ~= "table" then
			return false, schema.error("schema.Object(%s): not a table", x)
		end
		if #x > 0 then
			return false, schema.error("schema.Object(%s): has positional elements", x)
		end

		-- first check all keys match conditions
		for key, value in pairs(x) do
			local condition = condition_map[key] or additional_value_condition
			if not type(key) == "string" then
				return false, schema.error("schema.Object(%s): non-string key=%s, value=%s", x, key, value)
			end

			local result, err = condition(value)
			if not result then
				return false, schema.error("schema.Object(%s): invalid value=%s, key=%s, error=%s", x, value, key, err)
			end
		end

		-- then check all conditions matched by key (keys could be missing)
		for key, condition in pairs(condition_map) do
			local value = x[key]
			local result, err = condition(value)
			if not result then
				return false, schema.error(
					"schema.Object(%s): invalid value=%s / missing value for key=%s, error=%s", x, value, key, err)
			end
		end
		return true
	end
end
function schema.AnyOf(...)
	local conditions = {...}
	for _, condition in ipairs(conditions) do
		assert(schema.Function(condition))
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
		return false, schema.error("schema.AnyOf(%s): no match, match failures=\n%s", x, failures_str)
	end
end
function schema.AllOf(...)
	local conditions = {...}
	for _, condition in ipairs(conditions) do
		assert(schema.Function(condition))
	end

	return function(x)
		for _, condition in ipairs(conditions) do
			if not condition(x) then
				return false, schema.error("schema.AllOf(%s): no match", x)
			end
		end
		return true
	end
end
function schema.Enum(...)
	local values = {...}
	local values_str = table.concat(values, ", ")
	return function(x)
		for _, value in ipairs(values) do
			if x == value then
				return true
			end
		end
		return false, schema.error("schema.Enum(%s): no match, values={%s}", x, values_str)
	end
end
function schema.Const(const)
	return function(x)
		if x ~= const then
			return false, schema.error("schema.Const(%s): no match, const=%s", x, const)
		end
		return true
	end
end
function schema.Optional(condition)
	assert(schema.Function(condition))

	return function(x)
		if x == nil then
			return true
		end
		local result, err = condition(x)
		if result then
			return true
		end
		return false, schema.error("schema.Optional(%s): no match, err=%s", x, err)
	end
end
function schema.Check(condition)
	assert(schema.Function(condition))

	return function(x)
		local result, err = condition(x)
		if not result then
			return false, schema.error("schema.Check(%s): no match, err=%s, check=%s", x, err, string.dump(condition))
		end
		return true
	end
end
function schema.Integer(x)
	if type(x) == "number" and floor(x) == x and x >= integer_min and x <= integer_max then
		return true
	end
	return false, schema.error("schema.Integer(%s): no match", x)
end
function schema.NormalizedNumber(x)
	if type(x) == "number" and x >= 0 and x <= 1 then
		return true
	end
	return false, schema.error("schema.NormalizedNumber(%s): no match", x)
end
function schema.NonNegativeNumber(x)
	if type(x) == "number" and x >= 0 then
		return true
	end
	return false, schema.error("schema.NonNegativeNumber(%s): no match", x)
end
function schema.NonNegativeInteger(x)
	if type(x) == "number" and floor(x) == x and x >= 0 and x <= integer_max then
		return true
	end
	return false, schema.error("schema.NonNegativeInteger(%s): no match", x)
end
function schema.PositiveNumber(x)
	if type(x) == "number" and x > 0 then
		return true
	end
	return false, schema.error("schema.PositiveNumber(%s): no match", x)
end
function schema.PositiveInteger(x)
	if type(x) == "number" and floor(x) == x and x > 0 and x <= integer_max then
		return true
	end
	return false, schema.error("schema.PositiveInteger(%s): no match", x)
end
function schema.BoundedInteger(min, max)
	return function(x)
		if type(x) == "number" and floor(x) == x and x >= min and x <= max then
			return true
		end
		return false, schema.error("schema.BoundedInteger(%s): no match, min=%s, max=%s", x, min, max)
	end
end
local label_regex = "[a-z_][a-z0-9_]*"
function schema.LabelString(x)
	if type(x) == "string" and string.match(x, label_regex) == x then
		return true
	end
	return false, schema.error("schema.LabelString(%s): no match, regex=%s", x, label_regex)
end
function schema.NonEmptyString(x)
	if type(x) == "string" and #x > 0 then
		return true
	end
	return false, schema.error("schema.NonEmptyString(%s): no match", x)
end
function schema.NonEmptyArray(condition, opt_length)
	assert(schema.Function(condition))
	assert(schema.Optional(schema.PositiveInteger)(opt_length))
	local array_condition = schema.Array(condition, opt_length)

	return function(x)
		local result, err = array_condition(x)
		if not result then
			return false, schema.error("schema.NonEmptyArray(%s): no match, err=%s", x, err)
		end
		if #x == 0 then
			return false, schema.error("schema.NonEmptyArray(%s): empty", x)
		end
		return true
	end
end
function schema.PartialObject(condition_map)
	return schema.Object(condition_map, schema.Any)
end
local SerializableArray
local SerializableObject
function schema.Serializable(x)
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
	return false,  schema.error("schema.Serializable(%s): array_err=%s, object_err=%s", x, array_err, object_err)
end
SerializableArray = schema.Array(schema.Serializable)
SerializableObject = schema.Mapping(schema.String, schema.Serializable)
schema.SerializableArray = SerializableArray
schema.SerializableObject = SerializableObject
schema.AnyArray = schema.Array(schema.Any)
schema.AnyObject = schema.Object({}, schema.Any)

schema.tests = testing.add_suite("core.schema", {
	serialize_deserialize = function()
		local test_schema_values = {
			[schema.Any] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Boolean] = {true, false},
			[schema.String] = {"", "ye", string.dump(schema.Serializable)},
			[schema.Number] = {-1, 0, 2, 0.312, 2e5, 0.000001, math.huge, math.pi},
			[schema.Array(schema.Any)] = {{}, {1, 2}, {1, 2, "3"}, {{},{"yes", y = 2}}},
			[schema.Array(schema.Number)] = {{}, {1, 2}, {1, 2}},
			[schema.Mapping(schema.String, schema.Any)] =
				{{}, {a=2, b=3}, {a=2, b="ye"}, {a=2, b={{"ye", function() end}}}},
			[schema.Mapping(schema.String, schema.String)] = {{}, {a="b", b="c"}, {a="b"}},
			[schema.Object{a=schema.String, b=schema.Number}] = {{a="hello", b=2}},
			[schema.Enum("hello", "world")] = {"hello", "world"},
			[schema.Const("hello")] = {"hello"},
			[schema.Optional(schema.Number)] = {-1, 0, 2},  -- nil tested later, cant put in array
			[schema.Check(function(x) return x % 2 == 0 end)] = {0, 2, 4, 6},
			[schema.Integer] = {-1, 0, 2, 2e5, 2^24, -2^24},
			[schema.NormalizedNumber] = {0, 1, 0.001, 0.999, 0.4},
			[schema.NonNegativeNumber] = {0, 2, 0.312, 2e5, 0.000001, math.huge, math.pi},
			[schema.PositiveNumber] = {0.002, 1, 2, 0.312, 2e5, 0.000001, math.huge, math.pi},
			[schema.PositiveInteger] = {1, 2, 2e5},
			[schema.BoundedInteger(2, 4)] = {2, 3, 4},
			[schema.LabelString] = {"hello_world2", "a", "a2", "_az"},
			[schema.NonEmptyString] = {"y", "hello"},
			[schema.NonEmptyArray(schema.Number)] = {{1}, {1, 2, 3, 4, 5}},
			[schema.PartialObject{}] = {{}, {a=2}, {b=3}, {c=2, d={3, "4", {"5"}}}},
		}
		for value_schema, values in pairs(test_schema_values) do
			for _, value in ipairs(values) do
				local result, err = value_schema(value)
				if err then
					logging.error(err)
				end
				assert(err == nil)
				assert(result == true)
			end
		end

		local test_schema_failure_values = {
			[schema.None] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Null] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Boolean] = {-1, 0, 2, 0.312, "", "ye", function() end, {}, {1, 2}, {a=2, 3}},
			[schema.String] = {-1, 0, 2, 0.312, true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Number] = {"", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Array(schema.Any)] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {a = 2}},
			[schema.Array(schema.Number)] = {{{}}, {"ye"}, {1, 2, "3"}, {{},{"yes", y = 2}}},
			[schema.Mapping(schema.String, schema.Any)] =
				{-1, 0, 2, 0.312, "", "ye", true, false, function() end, {1, 2}, {a=2, 3}},
			[schema.Mapping(schema.String, schema.String)] =
				{-1, "", {1, 2}, {a=2, 3},  {a=2, b="ye"}, {a=2, b={{"ye", function() end}}}},
			[schema.None] = {-1, 0, 2, 0.312, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Enum("hello", "world")] = {"hello2", "world2", "hell", 2, true, 4, function() end},
			[schema.Const("hello")] = {"hell", 2, {}},
			[schema.Optional(schema.Number)] = {"", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.Check(function(x) return x % 2 == 0 end)] = {1, 3, 5},
			[schema.Integer] = {0.312, 0.000001, "", "ye", true, false, function() end, {}, {1, 2}, {a=2, 3}},
			[schema.NormalizedNumber] = {-1, -2, 3, 400, -2^24, 2^24, -0.01, 1.0001},
			[schema.NonNegativeNumber] = {-1, -2, -2^24, -math.huge, -math.pi},
			[schema.NonNegativeInteger] = {0.312, 0.000001, math.huge, math.pi},
			[schema.PositiveNumber] = {0, -1, -2, -2^24, -math.huge, -math.pi},
			[schema.PositiveInteger] = {0, 0.312, 0.000001, math.huge, math.pi},
			[schema.BoundedInteger(2, 4)] = {1, 5, 3.5 -1, 2000, 0.312, "", "ye", true, false, function() end, {}, {1, 2}},
			[schema.LabelString] =
				{-1, 0, 0.312, "YA", "2a", "2", "yep nope", " ", "yE", true, false, function() end, {}, {1, 2}},
			[schema.NonEmptyString] = {"", {}, {1}, {a = 1}, {nil}},
			[schema.NonEmptyArray(schema.Number)] = {"", "a", {}, {a = 1, 2}, {nil}, {"ye"}},
			[schema.PartialObject{}] = {-1, "", {1, 2}, {a=2, 3}},
		}
		for value_schema, failure_values in pairs(test_schema_failure_values) do
			for _, failure_value in ipairs(failure_values) do
				local result, err = value_schema(failure_value)
				assert(result == false)
				assert(type(err) == "string")
			end
		end

		-- extra cases for nil and none which can't be captured as array elements:
		assert(schema.Any())
		assert(schema.Any(nil))
		assert(schema.Null(nil))
		assert(schema.Optional(schema.Number)(nil))
		assert(not schema.None())
		assert(not schema.None(nil))
	end,
})

return schema
