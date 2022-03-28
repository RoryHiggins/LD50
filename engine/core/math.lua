local Math = {}

function Math.square_distance(x1, y1, x2, y2)
	local x_delta = (x2 - x1)
	local y_delta = (y2 - y1)
	return (x_delta * x_delta) + (y_delta * y_delta)
end
function Math.distance(x1, y1, x2, y2)
	local x_delta = (x2 - x1)
	local y_delta = (y2 - y1)
	return math.sqrt((x_delta * x_delta) + (y_delta * y_delta))
end

Math.epsilon = 2^-23
Math.integer_min = -2^24
Math.integer_max = 2^24

return Math
