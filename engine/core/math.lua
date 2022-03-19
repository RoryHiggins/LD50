local core_math = {}
function core_math.square_distance(x1, y1, x2, y2)
	local x_delta = (x2 - x1)
	local y_delta = (y2 - y1)
	return (x_delta * x_delta) + (y_delta * y_delta)
end
function core_math.distance(x1, y1, x2, y2)
	local x_delta = (x2 - x1)
	local y_delta = (y2 - y1)
	return math.sqrt((x_delta * x_delta) + (y_delta * y_delta))
end

return core_math
