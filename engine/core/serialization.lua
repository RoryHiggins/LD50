local Testing = require("engine/core/testing")

local Serialization = {}
function Serialization.get_escaped_string(str)
	return (
		str
		:gsub('\\', '\\\\')
		:gsub('\n', '\\n')
		:gsub('\r', '\\r')
		:gsub('\"', '\\"')
		:gsub("[^%w%p%s]", "?")
	)
end
function Serialization._get_strs(xs, out_strs, xs_type)
	out_strs = out_strs or {}
	xs_type = xs_type or type(xs)
	if xs_type == "table" then
		out_strs[#out_strs + 1] = "{"

		local added_first_key = false
		local int_val_count = 0
		local has_number_keys = #xs > 0
		local has_string_keys = false
		for key, x in pairs(xs) do
			if added_first_key then
				out_strs[#out_strs + 1] = ","
			else
				added_first_key = true
			end

			local key_type = type(key)
			if key_type == "number" then
				if math.floor(key) ~= key then
					error("cannot encode non-integer number key "..tostring(key))
				end
			elseif key_type == "string" then
				out_strs[#out_strs + 1] = "[\""
				out_strs[#out_strs + 1] = Serialization.get_escaped_string(key)
				out_strs[#out_strs + 1] = "\"]="
				has_string_keys = true
			else
				error("cannot encode key type "..tostring(key_type))
			end

			local x_type = type(x)
			if x_type == "string" then
				out_strs[#out_strs + 1] = "\""
				out_strs[#out_strs + 1] = Serialization.get_escaped_string(x)
				out_strs[#out_strs + 1] = "\""
			elseif x_type == "number" or x_type == "boolean" then
				out_strs[#out_strs + 1] = tostring(x)
				int_val_count = int_val_count + 1
			else
				Serialization._get_strs(x, out_strs, x_type)
			end
		end

		if has_string_keys and has_number_keys then
			-- possible but explicitly disabled as it's a bad fit for other encodings
			error("will not encode table with mixed key types")
		end

		if #xs > 0 and #xs < int_val_count then
			error("cannot encode sparse table")
		end

		out_strs[#out_strs + 1] = "}"
	elseif xs_type == "string" then
		out_strs[#out_strs + 1] = "\""
		out_strs[#out_strs + 1] = Serialization.get_escaped_string(xs)
		out_strs[#out_strs + 1] = "\""
	elseif xs_type == "number" or xs_type == "boolean" then
		out_strs[#out_strs + 1] = tostring(xs)
	else
		error("cannot encode type "..tostring(xs_type))
	end
	return out_strs
end
function Serialization.serialize(xs)
	return table.concat(Serialization._get_strs(xs))
end
function Serialization.deserialize(str)
	local success, result = pcall(load("return "..str))
	if not success then
		error("failed to deserialize "..str)
	end
	return result
end
Serialization.tests = Testing.add_suite("core.serialization", {
	serialize_deserialize = function()
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
			{"yes", "no"},
			{{"yes", "no"}},
			{1, 2, 3},
			{{}},
			{"yes", "no", 1, 2, 3, {}},
			{{1, 2, 3, 4, 5}, {"yes", "no"}},
			{a = {b = {c = {d = {e = {"f", "g", "h"}}}}}},
			{yes = {1, 2, 3}, no = {4, 5}},
		}
		for _, val in ipairs(test_values) do
			local serialized = Serialization.serialize(val)
			assert(type(serialized) == "string")

			local deserialized = Serialization.deserialize(serialized)
			assert(type(deserialized) == type(val))
			assert(Serialization.serialize(deserialized) == serialized)
		end
	end,
})

return Serialization
