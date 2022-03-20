local game = require("engine/engine/game")
local client = require("engine/engine/client")

local MinimalExample = game.Sys.new_metatable("minimal_example")
function MinimalExample:on_init()
	self.client = self.sim:require(client.GameSys)
end
function MinimalExample:on_step()
	local context = self.client.context

	context.vertex_array:add_triangle(0,0, 0,512, 512,0, 255,0,0,255)
	context.vertex_array:add_triangle(512,0, 0,512, 512,512, 0,255,0,255)

	local render_state_mouse_local = context.render_state_ortho_2d:copy():transform_view{
		translate_x = context.mouse.x - 256, translate_y = context.mouse.y - 256}
	context.renderer:draw_vertex_array{
		render_state = render_state_mouse_local, src = context.texture_atlas, vertex_array = context.vertex_array}
end

local game_sim = game.Game.new()
game_sim:require(MinimalExample)
game_sim:run()
