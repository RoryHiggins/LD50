local container = require("engine/core/container")

local Inputs = {}
Inputs.__index = Inputs
Inputs.defaults = {
	a = false,
	b = false,
	x = false,
	y = false,
	lx = 0,
	ly = 0,
}
Inputs.keys = container.get_keys(Inputs.defaults)
container.update_for_keys(Inputs, Inputs.defaults, Inputs.keys)
function Inputs:getEqual(other)
	return container.get_equal_for_keys(self.keys, self, other)
end
function Inputs.create()
	return setmetatable({}, Inputs)
end

local InputSys = {}
InputSys.Inputs = Inputs
-- TODO

return InputSys
