#include "game_math.h"
#include "projectile.h"
#include "game.h"

Projectile* gunLaser::Fire(Vector2 start_location, Level* level)
{
	int max_distance = 600;
	Projectile* laser = new Projectile(start_location - Vector2{ 0,16 }, 1, &(level->game->game_assets.spritesets["SHOT1"]), 0);
	MovementController* mc = new mcLinear(laser, { 1,0 }, max_distance);
	level->movement_controllers.push_back(mc); 
	level->projectiles.push_back(laser);
	Polygon p;
	p.vertices.push_back({ 0,16 });
	p.vertices.push_back({ 27,12 });
	p.vertices.push_back({ 31,16 });
	p.vertices.push_back({ 27,20 });
	laser->SetCollisionPolygon(p);
	mc->SetTravelTo(max_distance);
	return laser;
}
