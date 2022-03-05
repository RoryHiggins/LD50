local function main()
	local window = od.Window.new()
	window:init{}

	local frames = 0
	while window:step() do
		frames = frames + 1
	end
	window:destroy()

	print("frames", frames)
end

main()
