local level
local main_path
local enemy_path_1
local mc_main_path_follower
local game_coroutine
local event_handler
local dynamics = {}


function register_coroutine (object, func)
	dynamics[object] = {behavior = coroutine.create (func)}
	return dynamics[object].behavior
end

function resume_coroutine (object)

	if dynamics[object] ~= nil then

		if coroutine.status (dynamics[object].behavior) ~= 'dead' then

			coroutine.resume(dynamics[object].behavior, object)
		end
	end
end

function SpawnBot(x, y)
	print ("Spawning enemy")
	enemy = SpawnEnemy(level, "BOT1", {x,y}, "IDLE", 12)
	EnemySetLocationOffset(enemy, {-16,-16} )
	-- build the enemy's collision box
	collision_polygon = EnemyGetCollisionPolygon(enemy)
	PolyAddVertex(collision_polygon, { 1, 10 })
	PolyAddVertex(collision_polygon, { 4, 4 })
	PolyAddVertex(collision_polygon, { 8, 1 })
	PolyAddVertex(collision_polygon, { 23, 1 })
	PolyAddVertex(collision_polygon, { 27, 4 })
	PolyAddVertex(collision_polygon, { 30, 10 })
	PolyAddVertex(collision_polygon, { 30, 21 })
	PolyAddVertex(collision_polygon, { 27, 27 })
	PolyAddVertex(collision_polygon, { 23, 30 })
	PolyAddVertex(collision_polygon, { 8, 30 })
	PolyAddVertex(collision_polygon, { 4, 27 })
	PolyAddVertex(collision_polygon, { 1, 21 })
	return enemy
end

function Wave1Enemies(iterations)
	if iterations > 0 then
		enemy = SpawnBot(0,0)
		

		-- set the enemy on a movement path
		path_follower = CreatePathFollower(level, enemy, enemy_path_1, 0.05)
		total_distance = PathGetLength(enemy_path_1)
		Travel (path_follower, total_distance)
		OnAfter (event_handler, 1.0,  function() Wave1Enemies(iterations-1) end)
	end
end

function CameraRoutine (path_follower)
	local total_distance = PathGetLength(main_path)
	resume_func = function() resume_coroutine(mc_main_path_follower) end

	Travel(mc_main_path_follower, total_distance*0.1)
	OnTravelComplete (event_handler, mc_main_path_follower, resume_func)
	coroutine.yield()

	print ("10%")
	Wave1Enemies(10)
	
	Travel(mc_main_path_follower, total_distance*0.2)
	OnTravelComplete (event_handler, mc_main_path_follower, resume_func)
	coroutine.yield()
	
	print ("20%")
	Travel(mc_main_path_follower, total_distance*0.3)
	OnTravelComplete (event_handler, mc_main_path_follower, resume_func)
	coroutine.yield()

	-- pause for 10 seconds at the 30% mark
	
	print ("30% - pausing")
	OnAfter (event_handler, 6.0,  resume_func)
	coroutine.yield()


	print ("30% - resuming")
	Travel(mc_main_path_follower, total_distance*0.4)
	OnTravelComplete (event_handler, mc_main_path_follower, resume_func)
	coroutine.yield()
	print ("40%")

	Travel(mc_main_path_follower, total_distance*1)
	OnTravelComplete (event_handler, mc_main_path_follower, resume_func)
	coroutine.yield()
	print ("100%")
end


function StartGame (game)
	mc_main_path_follower = CreatePathFollower(level, CAMERA, main_path, 0.05)
	register_coroutine(mc_main_path_follower, CameraRoutine)
	resume_coroutine (mc_main_path_follower)
end

function BuildLevel (game)
	level = CreateLevel (game)


	-- creates an event handler that will be used to pass
	-- calls back and forth between c++ and lua
	event_handler = CreateEventHandler(level)

	-- backgrounds are rendered in the order they are added
	LevelAddBackground (level, "NEBULA")
	LevelAddBackground (level, "STARS1")
	LevelAddBackground (level, "STARS2")
	LevelAddBackground (level, "LEVEL1")

	main_path = CreatePath(level,{ 258, 418} ) -- creates a new path that originates at point (258, 418)
	PathBuildLine(main_path, { 1221, 415 })
	PathBuildCurve(main_path, { 1950, 453 })
	PathBuildControlPoint(main_path, { 2026, 429 })
	PathBuildControlPoint(main_path, { 2356, 154 })
	PathBuildControlPoint(main_path, { 2685, 305 })
	PathBuildControlPoint(main_path, { 2865, 549 })
	PathBuildControlPoint(main_path, { 3201, 508 })
	PathBuildLine(main_path, { 5406, 491})

	-- first enemy control path
	enemy_path_1 = CreatePath(level, {1244, 240})
	PathBuildLine(enemy_path_1, { 1160,329 })
	PathBuildCurve(enemy_path_1, {  1080,362 })
	PathBuildControlPoint(enemy_path_1, { 970,362 })
	PathBuildControlPoint(enemy_path_1, { 890,254 })
	PathBuildControlPoint(enemy_path_1, { 1116,184 })
	PathBuildLine(enemy_path_1, { 1113, 369 })
	PathBuildCurve(enemy_path_1, { 998,462 })
	PathBuildCurve(enemy_path_1, { 939,462 })
	PathBuildCurve(enemy_path_1, { 808,462 })
	PathBuildLine(enemy_path_1, { 431,462 })

	poly1 = CreateStaticCollisionPolygon(level)
	PolyAddVertex(poly1, { 1077, 208 })
	PolyAddVertex(poly1, { 1874, 26 })
	PolyAddVertex(poly1, { 2655, -1 })
	PolyAddVertex(poly1, { 2816, 15 })
	PolyAddVertex(poly1, { 3141, 99 })
	PolyAddVertex(poly1, { 3798, 115 })
	PolyAddVertex(poly1, { 3971, 113 })
	PolyAddVertex(poly1, { 4115, 178 })
	PolyAddVertex(poly1, { 4007, 236 })
	PolyAddVertex(poly1, { 3851, 292 })
	PolyAddVertex(poly1, { 3587, 417 })
	PolyAddVertex(poly1, { 2919, 408 })
	PolyAddVertex(poly1, { 2800, 364 })
	PolyAddVertex(poly1, { 2733, 124 })
	PolyAddVertex(poly1, { 2574, 33 })
	PolyAddVertex(poly1, { 2301, 33 })
	PolyAddVertex(poly1, { 2167, 190 })
	PolyAddVertex(poly1, { 2037, 287 })
	PolyAddVertex(poly1, { 1691, 258 })
	PolyAddVertex(poly1, { 1519, 286 })
	PolyAddVertex(poly1, { 1133, 270 })
	PolyAddVertex(poly1, { 1067, 215 })

	poly2 = CreateStaticCollisionPolygon(level)
	PolyAddVertex(poly2, { 882, 720 });
	PolyAddVertex(poly2, { 985, 638 });
	PolyAddVertex(poly2, { 1520, 638 });
	PolyAddVertex(poly2, { 1603, 578 });
	PolyAddVertex(poly2, { 1725, 580 });
	PolyAddVertex(poly2, { 1794, 662 });
	PolyAddVertex(poly2, { 2156, 664 });
	PolyAddVertex(poly2, { 2332, 415 });
	PolyAddVertex(poly2, { 2622, 416 });
	PolyAddVertex(poly2, { 2803, 575 });
	PolyAddVertex(poly2, { 2803, 649 });
	PolyAddVertex(poly2, { 3440, 658 });
	PolyAddVertex(poly2, { 3488, 716 });
	PolyAddVertex(poly2, { 3487, 823 });
	PolyAddVertex(poly2, { 880, 822 });
		
	poly3 = CreateStaticCollisionPolygon(level)
	PolyAddVertex(poly3, { 4382, 487 })
	PolyAddVertex(poly3, { 4508, 432 })
	PolyAddVertex(poly3, { 4726, 424 })
	PolyAddVertex(poly3, { 4875, 460 })
	PolyAddVertex(poly3, { 4859, 490 })
	PolyAddVertex(poly3, { 4505, 600 })
	PolyAddVertex(poly3, { 4380, 501 })

	LevelTriangulate(level) -- builds a list of triangles for collision detection
	LevelCalculatePathLengths(level) -- calculates the best approximation of all path lengths

	LevelSetMainPath(level, main_path)
	return level
end