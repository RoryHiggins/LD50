local Shim = require("engine/core/shim")

local Math = {}

local math_atan2 = Shim.atan2

local math_sqrt = math.sqrt
local math_floor = math.floor
local math_min = math.min
local math_max = math.max

function Math.square_distance(x1, y1, x2, y2)
	local x_delta = (x2 - x1)
	local y_delta = (y2 - y1)
	return (x_delta * x_delta) + (y_delta * y_delta)
end
function Math.distance(x1, y1, x2, y2)
	local x_delta = (x2 - x1)
	local y_delta = (y2 - y1)
	return math_sqrt((x_delta * x_delta) + (y_delta * y_delta))
end

function Math.direction(x, y)
	return math_atan2(x, -y)  -- -y as world is in y=down space
end
function Math.move_towards(x, y, dest_x, dest_y, max_speed)
	local dist_x, dist_y = (dest_x - x), (dest_y - y)
	local dist_sq = (dist_x * dist_x) + (dist_y * dist_y)
	local max_speed_sq = max_speed * max_speed

	if dist_sq <= max_speed_sq then
		return dest_x, dest_y
	end

	local dist_scale = math_sqrt(max_speed_sq) / math_sqrt(dist_sq)
	return (x + (dist_x * dist_scale)), (y + (dist_y * dist_scale))
end
function Math.round(x)
	return math_floor(x + 0.5)
end
function Math.clamp(x, min, max)
	return math_max(math_min(x, max), min)
end

Math.epsilon = 2^-23
Math.integer_min = -2^24
Math.integer_max = 2^24

return Math
