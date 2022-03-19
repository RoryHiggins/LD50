local shim = require("engine/core/shim")
local testing = require("engine/core/testing")

local container = {}
container.unpack = unpack or table.unpack  -- luacheck: globals unpack, ignore table

function container.table_get_keys(xs)
	assert(type(xs) == "table")

	local keys = {}
	for key, _ in pairs(xs) do
		keys[#keys + 1] = key
	end

	return keys
end
function container.table_get_equal_for_keys(keys, xs, ys)
	assert(type(xs) == "table")
	assert(type(ys) == "table")

	for _, key in ipairs(keys) do
		if xs[key] ~= ys[key] then
			return false
		end
	end

	return true
end
function container.object_update(xs, ys)
	assert(type(xs) == "table")
	assert(type(ys) == "table")

	for key, y in pairs(ys) do
		local y_type = type(y)
		if y_type == "number" or y_type == "string" or y_type == "nil" or y_type == "function" then
			xs[key] = y
		elseif y_type == "table" then
			container.object_update(xs[key], y)
		else
			error("unsupported type "..y_type)
		end
	end
end
function container.object_update_for_keys(keys, xs, ys)
	for _, key in ipairs(keys) do
		local y = ys[key]
		local y_type = type(y)
		if y_type == "number" or y_type == "string" or y_type == "nil" or y_type == "function" then
			xs[key] = y
		elseif y_type == "table" then
			container.object_update(xs[key], y)
		else
			error("unsupported type "..y_type)
		end
	end
end
function container.object_extend(xs, ys)
	assert(type(xs) == "table")
	assert(type(ys) == "table")
	for key, y in pairs(ys) do
		assert(xs[key] == nil)

		local y_type = type(y)
		if y_type == "number" or y_type == "string" or y_type == "function" then
			assert(xs[key] == nil)
			xs[key] = y
		elseif y_type == "table" then
			container.object_extend(xs[key], y)
		else
			error("unsupported type "..y_type)
		end
	end
end
function container.array_slice(xs, from, to)
	if from == nil then
		from = 1
	end
	if to == nil then
		to = #xs
	end

	return {shim.unpack(xs, from, to)}
end
function container.string_escape(str)
	return (
		str
		:gsub('\\', '\\\\')
		:gsub('\n', '\\n')
		:gsub('\r', '\\r')
		:gsub('\"', '\\"')
		:gsub("[^%w%p%s]", "?")
	)
end
function container.deep_copy(xs)
	local xs_type = type(xs)

	if xs_type == "number" or xs_type == "string" or xs_type == "nil" or xs_type == "function" then
		return xs
	elseif xs_type == "table" then
		local result = {}
		for key, x in pairs(xs) do
			result[key] = container.deep_copy(x)
		end
		return result
	else
		error("unsupported type "..xs_type)
	end
end
function container._get_comparable_strs(xs, out_strs, stack, indentation)
	local xs_type = type(xs)

	if xs_type == "string" then
		out_strs[#out_strs + 1] = "\""
		out_strs[#out_strs + 1] = container.string_escape(xs)
		out_strs[#out_strs + 1] = "\""
	elseif xs_type == "number" or xs_type == "boolean" then
		out_strs[#out_strs + 1] = tostring(xs)
	elseif xs_type == "table" then
		indentation = indentation or "\n"
		stack = stack or {}

		for _, seen in pairs(stack) do
			if xs == seen then
				out_strs[#out_strs + 1] = container.string_escape("<recursive="..tostring(xs)..">")
				return
			end
		end
		stack[#stack + 1] = xs

		local keys = container.table_get_keys(xs)
		table.sort(keys, function (a, b)
			return container.get_comparable_str(a) < container.get_comparable_str(b)
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
				out_strs[#out_strs + 1] = container.get_comparable_str(key)
				out_strs[#out_strs + 1] = ": "
			end

			container._get_comparable_strs(x, out_strs, stack, inner_indentation)
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
		out_strs[#out_strs + 1] = container.string_escape("<"..tostring(xs)..">")
		out_strs[#out_strs + 1] = "\""
	end

	return out_strs
end
function container.get_comparable_str(xs)
	return table.concat(container._get_comparable_strs(xs, {}))
end
function container.equal(xs, ys)
	return container.get_comparable_str(xs) == container.get_comparable_str(ys)
end
function container.assert_equal(xs, ys)
	local xs_str = container.get_comparable_str(xs)
	local ys_str = container.get_comparable_str(ys)
	assert(xs_str == ys_str, "container.assert_equal failed\n------\nxs=\n"..xs_str.."\nys=\n"..ys_str.."\n------\n")
end
container.tests = testing.add_suite("core.container", {
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
			container.get_comparable_str(val)
		end
	end,
	table_get_keys = function()
		local test_value_expected_pairs = {
			{{}, {}},
			{{5}, {1}},
			{{a = nil}, {}},
			{{a = 1}, {"a"}},
			{{10, 20, a = 30, c = nil, container}, {1, 2, 3, "a"}},
		}
		for _, pair in ipairs(test_value_expected_pairs) do
			local value, expected = pair[1], pair[2]
			container.assert_equal(container.table_get_keys(value), expected)
		end
	end,
	table_get_equal_for_keys = function()
		assert(container.table_get_equal_for_keys({1, 2}, {5, 10, 15}, {5, 10, 20}))
		assert(not container.table_get_equal_for_keys({1, 2, 3}, {5, 10, 15}, {5, 10, 20}))
	end
})

return container
