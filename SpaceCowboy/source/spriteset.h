#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "textures.h"
class Animation
{
public:
	std::vector<int> frames;
	int frame_rate;
	bool repeat = true;
	bool hold = false;
	void insert_frames(int int_arr[], int size);

	Animation(int frames = 0, int frame_arr[] = {}, int frame_rate = 3, bool repeat = false, bool hold = false) : frame_rate(frame_rate), repeat(repeat), hold(hold) {
		insert_frames(frame_arr, frames);
	}
};

struct AnimationTracker
{
	Animation* animation;
	std::vector<int>::iterator frame_it;
	Uint32 last_update;
	void reset() {
		if (animation != NULL) {
			frame_it = animation->frames.begin();
			last_update = SDL_GetTicks();
		}
	}
	void next() {
		//for (frame_it = animation->frames.begin(); frame_it != animation->frames.end(); ++frame_it)
		//	std::cout << ' ' << *frame_it;
		if (animation != NULL) {
			++frame_it;
			if (frame_it == animation->frames.end())
			{
				if (animation->repeat) reset();
				else if (animation->hold) --frame_it;
				else animation = NULL;
			}
			last_update = SDL_GetTicks();
		}
		
	}
	void tick() {
		if (animation != NULL) {
			Uint32 frame_delay = Uint32(1000 / animation->frame_rate);
			if (SDL_GetTicks() >= last_update + frame_delay) next();
		}
	}
	int get_sprite_num() {
		return *frame_it;
	}
};
class Spriteset
{
public:
	SDL_Texture* texture = NULL;
	std::unordered_map<std::string, Animation> animation_map;
	int tile_width = 22;
	int tile_height = 22;
	int tileset_width = 13;
	int tileset_height = 9;

	SDL_Rect get_sprite(int sprite_num);
	void draw_to(SDL_Renderer* renderer, int sprite_num, int x, int y);
	void add_animation(std::string anim_id, int frames,  int frame_array[], int frame_rate, bool repeat, bool hold);
	virtual void load(SDL_Renderer* renderer, std::string path, int tileWidth, int tileHeight);
	Animation* get_animation(std::string s);

};

class CustomSpriteset : public Spriteset
{
public:
	void load(SDL_Renderer* renderer, std::string path, int tileWidth, int tileHeight) override
	{
		texture = load_texture(renderer, path);
		tile_width = tileWidth;
		tile_height = tileHeight;
		SDL_QueryTexture(texture, NULL, NULL, &tileset_width, &tileset_height);
		tileset_width /= tile_width;
		tileset_height /= tile_height;
	}
	CustomSpriteset (SDL_Renderer* renderer, std::string path, int tileWidth, int tileHeight)
	{
		load(renderer, path, tileWidth, tileHeight);
	}
	CustomSpriteset ()
	{

	}
};



