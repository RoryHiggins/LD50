local container = {}
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
function container.get_keys(xs)
	assert(type(xs) == "table")

	local keys = {}
	for key, _ in pairs(xs) do
		keys[#keys + 1] = key
	end

	return keys
end
function container.get_equal_for_keys(keys, xs, ys)
	assert(type(xs) == "table")
	assert(type(ys) == "table")

	for _, key in ipairs(keys) do
		if xs[key] ~= ys[key] then
			return false
		end
	end

	return true
end
function container.update(xs, ys)
	assert(type(xs) == "table")
	assert(type(ys) == "table")

	for key, y in pairs(ys) do
		local y_type = type(y)
		if y_type == "number" or y_type == "string" or y_type == "nil" or y_type == "function" then
			xs[key] = y
		elseif y_type == "table" then
			container.update(xs[key], y)
		else
			error("unsupported type "..y_type)
		end
	end
end
function container.update_for_keys(keys, xs, ys)
	for _, key in ipairs(keys) do
		local y = ys[key]
		local y_type = type(y)
		if y_type == "number" or y_type == "string" or y_type == "nil" or y_type == "function" then
			xs[key] = y
		elseif y_type == "table" then
			container.update(xs[key], y)
		else
			error("unsupported type "..y_type)
		end
	end
end
function container.extend(xs, ys)
	assert(type(xs) == "table")
	assert(type(ys) == "table")
	for key, y in pairs(ys) do
		assert(xs[key] == nil)

		local y_type = type(y)
		if y_type == "number" or y_type == "string" or y_type == "function" then
			assert(xs[key] == nil)
			xs[key] = y
		elseif y_type == "table" then
			container.extend(xs[key], y)
		else
			error("unsupported type "..y_type)
		end
	end
end

return container
