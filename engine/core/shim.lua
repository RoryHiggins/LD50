local shim = {}
shim.unpack = unpack or table.unpack  -- luacheck: globals unpack, ignore table

return shim
