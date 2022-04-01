local Shim = require("engine/core/shim")
local Testing = require("engine/core/testing")
local Debugging = require("engine/core/debugging")
local Schema = require("engine/core/schema")

local debug_checks_enabled = Debugging.debug_checks_enabled

local Container = {}
function Container.deep_copy(xs)
	local xs_type = type(xs)

	if xs_type == "boolean" or xs_type == "number" or xs_type == "string" or xs_type == "nil" or xs_type == "function" then
		return xs
	elseif xs_type == "table" then
		local result = {}
		for key, x in pairs(xs) do
			result[key] = Container.deep_copy(x)
		end
		return result
	else
		error("cannot deep-copy type "..xs_type)
	end
end
function Container.get_keys(xs)
	if debug_checks_enabled then
		assert(Schema.Table(xs))
	end

	local keys = {}
	for key, _ in pairs(xs) do
		keys[#keys + 1] = key
	end

	return keys
end
function Container.get_values(xs)
	if debug_checks_enabled then
		assert(Schema.Table(xs))
	end

	local values = {}
	for _, value in pairs(xs) do
		values[#values + 1] = value
	end

	return values
end
function Container.update(xs, ...)
	local ys_srcs = {...}
	if debug_checks_enabled then
		assert(Schema.Table(xs))
		assert(Schema.NonEmptyArray(Schema.Table)(ys_srcs))
	end

	for _,ys in ipairs(ys_srcs) do
		local is_array = #xs > 0 or #ys > 0
		local iter, iter_invariant, iter_var
		if is_array then
			iter, iter_invariant, iter_var = ipairs(ys)
		else
			iter, iter_invariant, iter_var = pairs(ys)
		end

		for key, y in iter, iter_invariant, iter_var do
			local y_type = type(y)
			if y_type == "table" then
				local x = xs[key]
				if type(x) ~= "table" then
					x = {}
				end
				xs[key] = Container.update(x, y)
			else
				xs[key] = y
			end
		end
	end

	return xs
end
function Container.set_defaults(xs, ...)
	local ys_srcs = {...}

	if debug_checks_enabled then
		assert(Schema.Table(xs))
		assert(Schema.NonEmptyArray(Schema.Table)(ys_srcs))
	end

	for _,ys in ipairs(ys_srcs) do
		local is_array = #xs > 0 or #ys > 0
		local iter, iter_invariant, iter_var
		if is_array then
			iter, iter_invariant, iter_var = ipairs(ys)
		else
			iter, iter_invariant, iter_var = pairs(ys)
		end

		for key, y in iter, iter_invariant, iter_var do
			local y_type = type(y)
			if y_type == "table" then
				local x = xs[key]
				if type(x) ~= "table" then
					x = {}
				end
				xs[key] = Container.set_defaults(x, y)
			elseif y_type ~= "nil" then
				if xs[key] == nil then
					xs[key] = y
				end
			end
		end
	end

	return xs
end
function Container.array_slice(xs, from, to)
	if debug_checks_enabled then
		assert(Schema.AnyArray(xs))
		assert(Schema.BoundedInteger(0, #xs)(from))
		assert(Schema.Optional(Schema.BoundedInteger(from, #xs))(from))
	end

	if to == nil then
		to = #xs
	end

	return {Shim.unpack(xs, from, to)}
end
function Container.array_try_find(xs, key)
	for i, x in ipairs(xs) do
		if x == key then
			return i
		end
	end
end
function Container.string_escape(str)
	return (
		str
		:gsub('\\', '\\\\')
		:gsub('\n', '\\n')
		:gsub('\r', '\\r')
		:gsub('\"', '\\"')
		:gsub("[^%w%p%s]", "?")
	)
end
function Container._get_comparable_strs(xs, out_strs, stack, indentation)
	local xs_type = type(xs)

	if xs_type == "string" then
		out_strs[#out_strs + 1] = "\""
		out_strs[#out_strs + 1] = Container.string_escape(xs)
		out_strs[#out_strs + 1] = "\""
	elseif xs_type == "number" or xs_type == "boolean" then
		out_strs[#out_strs + 1] = tostring(xs)
	elseif xs_type == "table" then
		indentation = indentation or "\n"
		stack = stack or {}

		for _, seen in pairs(stack) do
			if xs == seen then
				out_strs[#out_strs + 1] = Container.string_escape("<recursive="..tostring(xs)..">")
				return
			end
		end
		stack[#stack + 1] = xs

		local keys = Container.get_keys(xs)
		table.sort(keys, function (a, b)
			return Container.get_comparable_str(a) < Container.get_comparable_str(b)
		end)

		local inner_indentation = indentation.."\t"

		local has_number_keys = #xs > 0
		local has_string_keys = not has_number_keys
		for _, key in ipairs(keys) do
			if type(key) ~= "number" or math.floor(key) ~= key then
				has_string_keys = true
				break
			end
		end

		if has_string_keys then
			out_strs[#out_strs + 1] = "{"
		else
			out_strs[#out_strs + 1] = "["
		end

		if #keys > 0 then
			out_strs[#out_strs + 1] = inner_indentation
		end

		local added_first_key = false

		for _, key in ipairs(keys) do
			local x = xs[key]

			if added_first_key then
				out_strs[#out_strs + 1] = ","
				out_strs[#out_strs + 1] = inner_indentation
			else
				added_first_key = true
			end

			if has_string_keys then
				out_strs[#out_strs + 1] = Container.get_comparable_str(key)
				out_strs[#out_strs + 1] = ": "
			end

			Container._get_comparable_strs(x, out_strs, stack, inner_indentation)
		end

		if #keys > 0 then
			out_strs[#out_strs + 1] = indentation
		end

		if has_string_keys then
			out_strs[#out_strs + 1] = "}"
		else
			out_strs[#out_strs + 1] = "]"
		end

	elseif xs_type == "nil" then
		out_strs[#out_strs + 1] = "null"
	else
		out_strs[#out_strs + 1] = "\""
		out_strs[#out_strs + 1] = Container.string_escape("<"..tostring(xs)..">")
		out_strs[#out_strs + 1] = "\""
	end

	return out_strs
end
function Container.get_comparable_str(xs)
	return table.concat(Container._get_comparable_strs(xs, {}))
end
function Container.equal(xs, ys)
	return Container.get_comparable_str(xs) == Container.get_comparable_str(ys)
end
function Container.assert_equal(xs, ys)
	local xs_str = Container.get_comparable_str(xs)
	local ys_str = Container.get_comparable_str(ys)
	assert(xs_str == ys_str, "Container.assert_equal failed\n------\nxs=\n"..xs_str.."\nys=\n"..ys_str.."\n------\n")
end
function Container.assert_not_equal(xs, ys)
	local xs_str = Container.get_comparable_str(xs)
	local ys_str = Container.get_comparable_str(ys)
	assert(xs_str ~= ys_str, "Container.assert_not_equal failed\n------\nxs=\n"..xs_str.."\nys=\n"..ys_str.."\n------\n")
end
Container.tests = Testing.add_suite("core.Container", {
	deep_copy = function()
		local test_tables = {
			{},
			{5},
			{a = nil},
			{a = 1},
			{"a"},
			{10, 20, a = 30, c = nil, Container},
			{1, 2, 3, "a"},
		}
		for _, value in ipairs(test_tables) do
			local copy = Container.deep_copy(value)
			Container.assert_equal(value, copy)

			value.inequality_test = "inequality"
			Container.assert_not_equal(value, copy)
			assert(copy.inequality_test == nil)
		end

		local test_primitives = {
			0,
			3.14159,
			true,
			false,
			"",
			"hello",
			function() end,
		}
		for _, value in ipairs(test_primitives) do
			local copy = Container.deep_copy(value)
			assert(value == copy)
		end

		local a = {a = 2, b = 3, c = {d = 4, e = 5}, 1}
		local b = Container.deep_copy(a)
		Container.assert_equal(a, b)
		a.a = 3

		Container.assert_not_equal(a, b)
		assert(a.a == 3)
		assert(b.a == 2)

		a.c.d = 5
		assert(a.c.d == 5)
		assert(b.c.d == 4)

		assert(Container.deep_copy(nil) == nil)
	end,
	get_keys = function()
		local test_value_expected_pairs = {
			{{}, {}},
			{{5}, {1}},
			{{a = nil}, {}},
			{{a = 1}, {"a"}},
			{{10, 20, a = 30, c = nil, Container}, {1, 2, 3, "a"}},
		}
		for _, pair in ipairs(test_value_expected_pairs) do
			local value, expected = pair[1], pair[2]
			Container.assert_equal(Container.get_keys(value), expected)
		end
	end,
	get_values = function()
		local test_value_expected_pairs = {
			{{}, {}},
			{{1}, {1}},
			{{a = 1, 2, 3}, {1, 2, 3}},
			{{a = 1}, {2}},
		}
		for _, pair in ipairs(test_value_expected_pairs) do
			local value, expected = pair[1], pair[2]
			Container.assert_equal(
				table.sort(Container.get_values(value)),
				table.sort(expected)
			)
		end
	end,
	update = function()
		local test_values_expected_tuples = {
			{{}, {}, {}},
			{{a = 1}, {b = 2}, {a = 1, b = 2}},
			{{a = 1, b = 1}, {b = 2}, {a = 1, b = 2}},
			{{}, {b = 2}, {b = 2}},
			{{a = 1, b = 1, c = {d = 1, e = 1}}, {b = 2, c = {e = 2}}, {a = 1, b = 2, c = {d = 1, e = 2}}},
			{{1, 2, 3, {4, 5}}, {}, {1, 2, 3, {4, 5}}},
			{{1, 2, 3, {4, 5}}, {1, 2, 3, {4, 5, 6}, 7}, {1, 2, 3, {4, 5, 6}, 7}},
			{{1, {a = 2}}, {2, {b = 3}}, {2, {a = 2, b = 3}}},
		}
		for _, tuple in ipairs(test_values_expected_tuples) do
			local defaults, updates, expected = tuple[1], tuple[2], tuple[3]
			Container.assert_equal(Container.update(defaults, updates), expected)
		end
	end,
	set_defaults = function()
		local test_values_expected_tuples = {
			{{}, {}, {}},
			{{a = 1}, {b = 2}, {a = 1, b = 2}},
			{{a = 1, b = 1}, {b = 2}, {a = 1, b = 1}},
			{{}, {b = 2}, {b = 2}},
			{{a = 1, b = 1, c = {d = 1, e = 1}}, {b = 2, c = {e = 2}}, {a = 1, b = 1, c = {d = 1, e = 1}}},
		}
		for _, tuple in ipairs(test_values_expected_tuples) do
			local defaults, updates, expected = tuple[1], tuple[2], tuple[3]
			Container.assert_equal(Container.set_defaults(defaults, updates), expected)
		end
	end,
	array_slice = function()
		local test_value_expected_tuples = {
			{{}, 0, 0, {}},
			{{5}, 1, 1, {5}},
			{{1, 2, 3, 4}, 1, 2, {1, 2}},
			{{1, 2, 3, 4}, 2, 3, {2, 3}},
			{{1, 2, 3, 4}, 1, 4, {1, 2, 3, 4}},
		}
		for _, tuple in ipairs(test_value_expected_tuples) do
			local value, from, to, expected = tuple[1], tuple[2], tuple[3], tuple[4]
			Container.assert_equal(Container.array_slice(value, from, to), expected)
		end
	end,
	array_try_find = function()
		assert(Container.array_try_find({1, 2, 3, 4, "5"}, 1) == 1)
		assert(Container.array_try_find({1, 2, 3, 4, "5"}, 4) == 4)
		assert(Container.array_try_find({1, 2, 3, 4, "5"}, "5") == 5)
		assert(Container.array_try_find({1, 2, 3, 4, "5"}, 0) == nil)
		assert(Container.array_try_find({1, 2, 3, 4, "5"}, 5) == nil)
	end,
	string_escape = function()
		local test_value_expected_pairs = {
			{"hello", "hello"},
			{'"hello"', '\\"hello\\"'},
			{"C:\\", "C:\\\\"},
			{"hello\n World", "hello\\n World"},
		}
		for _, pair in ipairs(test_value_expected_pairs) do
			local value, expected = pair[1], pair[2]
			Container.assert_equal(Container.string_escape(value), expected)
		end
	end,
	get_comparable_str = function()
		local recursive = {1, 2, 3, yes="no", a={1, 2, "ye"}}
		recursive.recursive = recursive

		local test_values = {
			0,
			-0,
			-1024,
			0.5,
			4444444444,
			"",
			"yes",
			"\"\n\"\t",
			{},
			{{}},
			{"yes", "no"},
			{{"yes", "no"}},
			{1, 2, 3},
			{"yes", "no", 1, 2, 3, {}},
			{{1, 2, 3, 4, 5}, {"yes", "no"}},
			{yes = {1, 2, 3}, no = {4, 5}},
			{a = {b = {c = {d = {e = {"f", "g", "h"}}}}}},
			{"yes", "no", 1, 2, 3, a = "b", c = {d = {"f"}, "g"}},
			{[{1, 2, 3, a = 4}] = {5, 6, 7, b = 8}},
			{function() end, "yes", 2, {2, [function() end] = 2, 3, no = 5}},
			recursive,
			nil,
		}
		for _, val in ipairs(test_values) do
			Container.get_comparable_str(val)
		end
	end,
})

return Container
