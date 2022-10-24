#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <string>


class Tileset
{
public:
	SDL_Texture* texture = NULL;
	int tile_width = 32;
	int tile_height = 32;
	int tileset_width = 21;
	int tileset_height = 23;
	
	SDL_Rect get_tile(int tile_num);
	void draw_to(SDL_Renderer* renderer, int tile_num,  int x, int y);
	virtual void load(SDL_Renderer* renderer, std::string path);

};
