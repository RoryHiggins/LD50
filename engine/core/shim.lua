local Shim = {}
Shim.unpack = unpack or table.unpack  -- luacheck: globals unpack, ignore table

Shim.atan2 = math.atan2 or math.atan  -- luacheck: globals math

return Shim
