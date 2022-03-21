local Shim = {}
Shim.unpack = unpack or table.unpack  -- luacheck: globals unpack, ignore table

return Shim
