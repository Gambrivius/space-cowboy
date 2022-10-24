#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include "game_math.h"
#include "spriteset.h"
#include "level.h"

// forward declaration of TileMap from game_assets.h
// TileMaps need a list of entities

class TileMap;

class Entity : public Dynamic {
private:
	Polygon collision_polygon;
	std::vector<Triangle> collision_triangles;

public:
	SDL_Texture* texture = NULL;
	Level* level = NULL;
	Spriteset* spriteset = NULL;
	AnimationTracker animation_tracker;
	Vector2 velocity;
	int hp;

	float max_velocity = 0.4f;
	float acceleration = 0.2f;
	float resistance = 0.01f;
	Vector2 v_offset = { -32, -32 };
	std::string animation_id;
	int sprite_num = 0;
	
	void set_position(Vector2 pos) {
		location = pos;
	}
	bool move(Vector2 new_pos);
	void move_and_glide(Vector2 v, Vector2* new_velocity);
	void set_animation(std::string id);
	void cancel_animation();
	void set_collision_polygon(Polygon p);
	bool is_colliding_against(std::vector<Triangle>* triangles);
	bool would_collide_against(std::vector<Triangle>* triangles, Vector2 new_location);
	std::vector<Triangle> get_translated_triangles(Vector2 offset);
};