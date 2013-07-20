print("======================================================================")
print("Hello from Lua!")
print("======================================================================")

require("math")

-- Camera is currently a first person perspective camera
-- methods:
--  c:setPosition(x, y, z)
--  c:setYawPitch(yaw [, pitch])
--  x, y, z = c:getPosition()
--  yaw, pitch = c:getYawPitch()
fpcam = Camera()

-- Scene contains objects within a scene as well as lights
-- its constructor requires a Camera instance
-- methods:
--   refID = s:enters(actor)
--   s:exits(refID)
--   s:configureLight(lightIndex, pos_x, pos_y, pos_z, dir_x, dir_y, dir_z, 
--              beamHalfAngle, constantAttenuation, linearAtt, quadraticAtt)
--   s:moveLight(lightIndex, pos_x, pos_y, pos_z [, dir_x, dir_y, dir_z])
--   modelID = s:getModel(modelFileName) 
--      modelFileName is any 3d object file name; the data is cached and reused
--      You need modelID to initialize an Actor object, see below.
scene = Scene(fpcam)
if not scene then
  print("Scene not initialized? Shouldn't happen.")
end


-- Actor is a 3D object with a world position and orientation
-- there's an animation frame as well (implementation pending...)
-- The constructor requires a modelID and takes optional correction arguments:
--   a = Actor(modelID [, axis_x, axis_y, axis_z, angle [, scale 
--       [, x_offset, y_offset, z_offset]]])
-- methods:
--   a:moveTo(x, y, z)
--   a:setRollPitchYaw(roll, pitch, yaw)
--   TODO: more methods :|

-- the house is our token 3DS model and it's all wrong
-- rotations are in radians, btw
house = Actor(scene:getModel("LPBuildX13r_3ds.3ds"), 1.0, 0.0, 0.0, math.pi/2, 0.15, 0, 4.5001, 0)
scene:enters(house)
house:moveTo(-10, 0, 15)

-- ducks float too high up
newDuck = function ()
  return Actor(scene:getModel("duck.obj"), 1, 0, 0, 0, 1, 0, -0.2, 0);
end
duck = newDuck()
scene:enters(duck)
duck:moveTo(-3, 0, 3)

duck2 = newDuck()
scene:enters(duck2)
duck2:moveTo(3, 0, 3)

-- 121 ducks
-- if you remove the stepping value of 2, it's 506 ducks
ducks = {}
xnum = 23
ynum = 22

function duckLoop(work)
	for x = 1, xnum, 2 do
		for y = 1, ynum, 2 do
			work(x,y)
		end
	end
end

duckLoop(function(x,y)
			if (ducks[y] == nil) then ducks[y]={} end
			d = newDuck()
			scene:enters(d)
			d:moveTo(x*4 - xnum*2, 2, y*4 - ynum*2)
			-- ducks[#ducks + 1] = d
			ducks[y][x] = d
end)

torus = Actor(scene:getModel("torus.obj"), 1, 0, 0, 0, 1)
scene:enters(torus)
torus:moveTo(0, 2.5, 3)

bigTorus = Actor(scene:getModel("torus.obj"), 1, 0, 0, -math.pi/4, 1.5) -- the torus is tilted slightly and it's obnoxious
scene:enters(bigTorus)
bigTorus:moveTo(0, 2.5, 3)

tree = Actor(scene:getModel("spooky_tree.obj"))
scene:enters(tree)
tree:moveTo(-7, 0, 6)

floor = Actor(scene:getModel("floor.obj"))
scene:enters(floor)
floor:moveTo(0, 0, 10)

-- Ensign Chekov is too big
-- chekov = Actor(scene:getModel("Chekov.obj"), 1, 0, 0, 0, 0.55)
-- a zero-degree rotation is an identity or whatever
-- scene:enters(chekov)
-- chekov:moveTo(0, 0, 7)

-- the C++ code will normalize this vector for you:
function directionalLight()
	-- scene:pointMoonlight(0.1, -0.2, 1.0)
	scene:pointMoonlight(-0.894263, -0.359565, 0.266471)   -- good value for GrimmNight_cube.dds
end

scene:configureLight(1, -7.2, 3, 6, 0.000001, -1, 0, math.pi * (30/180), 1, 0, 0 )
-- scene:moveLight(1, -7.2, 3, 6, 0, -1, 0)

i = 0

-- tada.

function duckWave(x,y) 
	ducks[y][x]:moveTo(x*4-xnum*2, 2 + math.sin(x+timer:now())/4 + math.cos(y+timer:now())/4, y*4-ynum*2+0.5) 
end


-- update() gets called every frame but more hooks are probably needed to do
-- anything really interesting:

function update(now, timeSinceLastFrame)
	scene:updateFlashlight(0) -- light 0 is a flashlight

	-- this call also updates the cascading shadow map setup:
	directionalLight()

	duck:moveTo(-3, math.sin(now), 3) -- bobbing duck

	torus:setRollPitchYaw(0, now*1.1, 0) -- spinning torus
	bigTorus:setRollPitchYaw(-now, 0, 0)
	duck2:moveTo(0, 2, 3)
	duck2:setRollPitchYaw(0, 0, now*2)

	duckLoop(duckWave)
end
