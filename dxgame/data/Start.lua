print("======================================================================")
print("Hello from Lua!")
print("======================================================================")

require("math")

scene = Scene()

duck = Actor(scene:getModel("duck.obj"))
scene:enters(duck)

duck2 = Actor(scene:getModel("duck.obj"))
scene:enters(duck2)

duck2:moveTo(3, 0, 3)

function update(now)
	duck:moveTo(0, math.sin(now), 0)
end
