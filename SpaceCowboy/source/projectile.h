#pragma once
#include "game_math.h"
#include "spriteset.h"
#include "game_assets.h"
#include "level.h"

class Projectile : public Dynamic
{
public:
	Spriteset* spriteset = NULL;
	AnimationTracker animation_tracker;
	int damage;
	std::string animation_id;
	int sprite_num = 0;
	Projectile(Vector2 Location, int Damage, Spriteset* Sprite, int SpriteNum)
	{
		location = Location;
		damage = Damage;
		spriteset = Sprite;
		sprite_num = SpriteNum;
	}
	~Projectile()
	{

	}
};

class Gun
{
	Uint64 lastFire;
	float fireDelay;
public:
	virtual Uint64 GetFireDelay() = 0;
	bool isReady()
	{
		return SDL_GetPerformanceCounter() > lastFire+ GetFireDelay();
	}
	Projectile* fireIfReady(Vector2 start_location, Level* level)
	{
		if (isReady())
		{
			lastFire = SDL_GetPerformanceCounter();
			return Fire(start_location, level);
		}
		return nullptr;
	}

	virtual Projectile* Fire(Vector2 start_location, Level* level) = 0;
};

class gunLaser : public Gun
{
	float fireDelay = 0.1;
public:
	Uint64 GetFireDelay() override {
		return fireDelay * SDL_GetPerformanceFrequency();
	}
	Projectile* Fire(Vector2 start_location, Level* level) override;
};