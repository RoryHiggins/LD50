local json = require("engine/lib/json/json")
local Engine = require("engine/engine")
local Core = Engine.Core

local math_floor = math.floor

local default_tileset = {
	filename = "./ld50/data/sprites.png",
	file_type = "png",
	tile_width = 8,
	tile_height = 8,
	tiles = {
		player = {0, 8},
		tree = {8, 8},

		-- terrain
		water = {8, 0},
		sand = {16, 0},
		grass = {24, 0},

		-- decoration
		dirt_1 = {0, 24},
		dirt_2 = {8, 24},
		dirt_3 = {16, 24},
		dirt_4 = {24, 24},
		dirt_5 = {32, 24},
		dirt_6 = {40, 24},
		dirt_7 = {48, 24},
		dirt_8 = {56, 24},
		terrain_mix_n_1 = {16, 72},
		terrain_mix_n_2 = {24, 72},
		terrain_mix_n_3 = {32, 72},
		terrain_mix_n_4 = {40, 72},
		terrain_mix_n_5 = {48, 72},
		terrain_mix_s_1 = {16, 80},
		terrain_mix_s_2 = {24, 80},
		terrain_mix_s_3 = {32, 80},
		terrain_mix_s_4 = {40, 80},
		terrain_mix_s_5 = {48, 80},
		terrain_mix_w_1 = {0, 72},
		terrain_mix_w_2 = {0, 80},
		terrain_mix_w_3 = {0, 88},
		terrain_mix_w_4 = {0, 96},
		terrain_mix_w_5 = {0, 104},
		terrain_mix_e_1 = {8, 72},
		terrain_mix_e_2 = {8, 80},
		terrain_mix_e_3 = {8, 88},
		terrain_mix_e_4 = {8, 96},
		terrain_mix_e_5 = {8, 104},
	},
	animations = {
		tide_nw = {{0, 32}, {32, 32}},
		tide_n = {{8, 32}, {40, 32}},
		tide_ne = {{16, 32}, {48, 32}},
		tide_w = {{0, 40}, {32, 40}},
		tide_e = {{16, 40}, {48, 40}},
		tide_sw = {{0, 48}, {32, 48}},
		tide_s = {{8, 48}, {40, 48}},
		tide_se = {{16, 48}, {48, 48}},

		tide_nsw = {{0, 56}, {32, 56}},
		tide_ns = {{8, 56}, {40, 56}},
		tide_nse = {{16, 56}, {48, 56}},

		tide_nwe = {{24, 32}, {56, 32}},
		tide_we = {{24, 40}, {56, 40}},
		tide_swe = {{24, 48}, {56, 48}},

		tide_nswe = {{0, 64}, {32, 64}},
	},
	tile_defaults = {
		-- entities
		player = {
			tags = {solid = true},
		},
		tree = {
			tags = {solid = true},
		},

		-- terrain
		sand = {
			tags = {terrain = true},
			dirt_color = {191, 121, 88, 255},
			tide_color = {255, 255, 255, 255},
			terrain_color = {244, 204, 161, 255},
		},
		grass = {
			tags = {terrain = true},
			dirt_color = {57, 123, 68, 255},
			terrain_color = {113, 170, 52, 255},
		},
		water = {
			tags = {solid = true},
		},
	},
	bounds_indexed_tags = {"solid", "terrain", "water", "sand", "grass", "player"},
	loaded = false,
	index_tile_name = {}
}

local Data = {}
Data.WorldSys = Engine.World.Sys.new_metatable("data")
function Data.WorldSys:load_tileset(tileset)
	if tileset.loaded == true then
		return tileset
	end

	tileset.index_tile = {}
	local tilemap_width, tilemap_height = self._image:filename_get_size(
		tileset.filename, tileset.file_type
	)
	local cols, rows = (tilemap_width / tileset.tile_width), (tilemap_height / tileset.tile_height)
	assert(Core.Schema.PositiveInteger(cols))
	assert(Core.Schema.PositiveInteger(rows))
	assert(Core.Schema.SerializableObject(tileset))

	for tile_name, uv in pairs(tileset.tiles) do
		local col = (uv[1] / tileset.tile_width)
		local row = (uv[2] / tileset.tile_height)
		assert(Core.Schema.BoundedInteger(col, 0, cols - 1))
		assert(Core.Schema.BoundedInteger(row, 0, rows - 1))
		local index = col + (row * cols)
		tileset.index_tile_name[index] = tile_name
	end

	self._image:set_batch(
		tileset.tiles, tileset.filename, tileset.file_type, tileset.tile_width, tileset.tile_height
	)
	for tile_name, _ in pairs(tileset.tiles) do
		self._template:set_defaults(tile_name, {
			width = tileset.tile_width,
			height = tileset.tile_height,
			tags = {[tile_name] = true},
			image_name = tile_name,
		})
	end
	for anim_name, frame_bounds in pairs(tileset.animations) do
		self._animation:set_frames(
			anim_name, frame_bounds, tileset.filename, tileset.file_type, tileset.tile_width, tileset.tile_height
		)
	end

	for tile_name, defaults in pairs(tileset.tile_defaults) do
		self._template:update(tile_name, defaults)
	end

	self._entity:tag_bounds_index_add(tileset.bounds_indexed_tags)

	return tileset
end
function Data.WorldSys:load_tilemap(filename, tileset)
	local file, err = io.open(filename, "r")
	if file == nil then
		Core.Logging.info("failed to open save file for reading, filename=%s, err=%s", filename, err)
		return false
	end
	local tilemap_json = file:read("*a")
	file:close()

	local tilemap = json.decode(tilemap_json)
	assert(tilemap.tilewidth == tileset.tile_width)
	assert(tilemap.tileheight == tileset.tile_height)
	assert(tilemap.orientation == "orthogonal")
	assert(tilemap.renderorder == "right-down")
	assert(tilemap.type == "map")
	assert(tilemap.infinite == false)
	assert(tilemap.compressionlevel == -1)

	local index_tile_name = tileset.index_tile_name
	local tile_width, tile_height = tileset.tile_width, tileset.tile_height
	for i, layer in ipairs(tilemap.layers) do
		assert(layer.type == "tilelayer")

		local cols = layer.width
		local start_x = layer.x * tile_width
		local start_y = layer.y * tile_height
		local a = math.floor(255 * layer.opacity)
		if layer.visible == false then
			a = 0
		end
		for j, tile_index in ipairs(layer.data) do
			tile_index = tile_index - 1

			local tile_name = index_tile_name[tile_index]
			if tile_name ~= nil then
				local col = (j - 1) % cols
				local row = math_floor((j - 1) / cols)
				local x = start_x + (col * tile_width)
				local y = start_y + (row * tile_height)

				self._template:instantiate(tile_name, {x = x, y = y, z = (-i * 100) + 1, a = a})
			end
		end
	end
end
function Data.WorldSys:on_init()
	self._image = self.sim:require(Engine.Image.WorldSys)
	self._animation = self.sim:require(Engine.Animation.WorldSys)
	self._template = self.sim:require(Engine.Template.WorldSys)
	self._entity = self.sim:require(Engine.Entity.WorldSys)

	self.tileset = self:load_tileset(default_tileset)
end
function Data.WorldSys:on_start_begin()
	self:load_tilemap("./ld50/data/map.json", self.tileset)
	-- self:load_tilemap("./ld50/data/map_min.json", self.tileset)
end

Data.GameSys = Engine.Game.Sys.new_metatable("data")
Data.GameSys.WorldSys = Data.WorldSys

return Data
