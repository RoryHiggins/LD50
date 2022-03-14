local container = require("engine/core/container")
local testing = require("engine/core/testing")

local json = {}
function json._get_strs(xs, out_strs)
	local xs_type = type(xs)

	if xs_type == "string" then
		out_strs[#out_strs + 1] = "\""
		out_strs[#out_strs + 1] = container.string_escape(xs)
		out_strs[#out_strs + 1] = "\""
	elseif xs_type == "number" or xs_type == "boolean" then
		out_strs[#out_strs + 1] = tostring(xs)
	elseif xs_type == "table" then
		local keys = container.table_get_keys(xs)

		local added_first_key = false
		local has_number_keys = (#xs > 0)
		local has_string_keys = not has_number_keys

		if has_string_keys then
			out_strs[#out_strs + 1] = "{"
		else
			out_strs[#out_strs + 1] = "["
		end

		for _, key in ipairs(keys) do
			local x = xs[key]

			local key_type = type(key)
			if key_type == "number" then
				has_number_keys = true
				if math.floor(key) ~= key then
					error("cannot encode non-integer number key "..tostring(key))
				end
			elseif key_type == "string" then
				has_string_keys = true
			else
				error("cannot encode key type "..tostring(key_type))
			end

			if has_number_keys and has_string_keys then
				error("cannot encode table with mix of number and string keys")
			end

			if added_first_key then
				out_strs[#out_strs + 1] = ","
			else
				added_first_key = true
			end

			if has_string_keys then
				out_strs[#out_strs + 1] = "\""
				out_strs[#out_strs + 1] = container.string_escape(key)
				out_strs[#out_strs + 1] = "\":"
			end

			json._get_strs(x, out_strs)
		end

		if has_string_keys then
			out_strs[#out_strs + 1] = "}"
		else
			out_strs[#out_strs + 1] = "]"
		end
	elseif xs_type == "nil" then
		out_strs[#out_strs + 1] = "null"
	else
		error("cannot encode type "..tostring(xs_type))
	end
	return out_strs
end

function json.encode(xs)
	return table.concat(json._get_strs(xs, {}))
end

testing.add_suite("core.json", {
	encode = function()
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
		}
		for _, val in ipairs(test_values) do
			json.encode(val)
		end
	end,
	encode_invalid_fails = function()
		local recursive = {1, 2, 3, yes="no", a={1, 2, "ye"}}
		recursive.recursive = recursive

		local test_values = {
			{"yes", "no", 1, 2, 3, a = "b", c = {d = {"f"}, "g"}},
			{[{1, 2, 3, a = 4}] = {5, 6, 7, b = 8}},
			{function() end, "yes", 2, {2, [function() end] = 2, 3, no = 5}},
			recursive,
		}
		for _, val in ipairs(test_values) do
			testing.assert_fails(function()
				json.encode(val)
			end)
		end
	end,
})

return json
