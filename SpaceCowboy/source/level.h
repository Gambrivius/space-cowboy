#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "game_math.h"
#include "scripting.h"

class Projectile;
class Game;
class Entity;

class Level
{
public:


	std::vector<std::string> backgrounds;
	std::vector<std::string> foregrounds;
	Path* main_path; // DONT FORGET: it looks like when paths gets appended to, it gets
						// copied to a new memory location and this pointer is no longer valid
						// may need a different container than vector if we need
						// persistent pointers.  This will break the LUA also...

						// this was easily remedied by converting paths to a vector of pointers
						// but still need to implement a way to deconstruct every object
						// that gets dynamically constructed...
	Game* game;
	Vector2 origin;
	std::vector<Entity*> entities;
	std::vector<Projectile*> projectiles;
	std::vector<Entity*>::iterator entity_it = entities.begin();
	std::vector<Rect> collision_boxes;
	std::vector<Polygon> collision_polygons;
	std::vector<Path*> paths;
	std::vector<Triangle> collision_triangles;

	std::vector<MovementController*> movement_controllers;
	std::vector<MovementController*> future_movement_controllers;

	std::vector<LuaEventHandler*> lua_event_handlers;
	virtual void init() = 0;
	Entity* SpawnEntity(Spriteset* spriteset, Vector2 location, std::string default_animation);
	void CheckProjectileCollisions();
	void UpdateMovementControllers( float delta_time);
	void PrettyPrint()
	{
		for (Path* path : paths)
		{
			path->PrettyPrint();
		}
		for (Polygon& p : collision_polygons)
		{
			p.PrettyPrint();
		}
	}
	void Triangulate()
	{
		collision_triangles.clear();
		for (Polygon& p : collision_polygons)
		{
			for (Triangle t : p.Triangulate())
			{
				collision_triangles.push_back(t);
			}
		}
	}
	void CalculatePathLengths()
	{
		for (Path* p : paths)
		{
			p->UpdateLengths();
		}
	}
};
class CustomLevel : public Level
{
public:
	void init()
	{
		// this class is completely implemented by scripts
	}
};

class Level1 : public Level
{
public:
	void init()
	{
		backgrounds = {"NEBULA", "STARS1", "STARS2", "LEVEL1"};
		foregrounds = {};
		paths.emplace_back( new Path());
		main_path = paths.back();


		main_path->SetFirstBuildPoint({ 258, 418 });
		main_path->BuildLine({ 1221, 415 });
		main_path->BuildCurve({ 1950.5818, 453.3930 });
		main_path->AddBezierPoint({ 2026, 429 });
		main_path->AddBezierPoint({ 2356, 154 });
		main_path->AddBezierPoint({ 2685, 305 });
		main_path->AddBezierPoint({ 2865, 549 });
		main_path->AddBezierPoint({ 3201, 508 });
		main_path->BuildLine({ 5406.5708, 491.1837 });
		main_path->UpdateLengths();
		collision_polygons.emplace_back();
		Polygon* poly1 = &collision_polygons.back();
		poly1->vertices.push_back({ 1077, 208 });
		poly1->vertices.push_back({ 1874, 26 });
		poly1->vertices.push_back({ 2655, -1 });
		poly1->vertices.push_back({ 2816, 15 });
		poly1->vertices.push_back({ 3141, 99 });
		poly1->vertices.push_back({ 3798, 115 });
		poly1->vertices.push_back({ 3971, 113 });
		poly1->vertices.push_back({ 4115, 178 });
		poly1->vertices.push_back({ 4007, 236 });
		poly1->vertices.push_back({ 3851, 292 });
		poly1->vertices.push_back({ 3587, 417 });
		poly1->vertices.push_back({ 2919, 408 });
		poly1->vertices.push_back({ 2800, 364 });
		poly1->vertices.push_back({ 2733, 124 });
		poly1->vertices.push_back({ 2574, 33 });
		poly1->vertices.push_back({ 2301, 33 });
		poly1->vertices.push_back({ 2167, 190 });
		poly1->vertices.push_back({ 2037, 287 });
		poly1->vertices.push_back({ 1691, 258 });
		poly1->vertices.push_back({ 1519, 286 });
		poly1->vertices.push_back({ 1133, 270 });
		poly1->vertices.push_back({ 1067, 215 });
		
		collision_polygons.emplace_back();
		
		Polygon* poly2 = &collision_polygons.back();
		poly2->vertices.push_back({ 882, 720 });
		poly2->vertices.push_back({ 985, 638 });
		poly2->vertices.push_back({ 1520, 638 });
		poly2->vertices.push_back({ 1603, 578 });
		poly2->vertices.push_back({ 1725, 580 });
		poly2->vertices.push_back({ 1794, 662 });
		poly2->vertices.push_back({ 2156, 664 });
		poly2->vertices.push_back({ 2332, 415 });
		poly2->vertices.push_back({ 2622, 416 });
		poly2->vertices.push_back({ 2803, 575 });
		poly2->vertices.push_back({ 2803, 649 });
		poly2->vertices.push_back({ 3440, 658 });
		poly2->vertices.push_back({ 3488, 716 });
		poly2->vertices.push_back({ 3487, 823 });
		poly2->vertices.push_back({ 880, 822 });
		collision_polygons.emplace_back();
		Polygon* poly3 = &collision_polygons.back();
		poly3->vertices.push_back({ 4382, 487 });
		poly3->vertices.push_back({ 4508, 432 });
		poly3->vertices.push_back({ 4726, 424 });
		poly3->vertices.push_back({ 4875, 460 });
		poly3->vertices.push_back({ 4859, 490 });
		poly3->vertices.push_back({ 4505, 600 });
		poly3->vertices.push_back({ 4380, 501 });
		for (Polygon& p : collision_polygons)
		{
			for (Triangle t : p.Triangulate())
			{
				collision_triangles.push_back(t);
			}
		}
		

	}
};