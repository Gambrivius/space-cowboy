#include <SDL.h>
#include <stdio.h>
#include <string>
#include "tileset.h"
#include "textures.h"

SDL_Rect Tileset::get_tile(int tile_num) {
	SDL_Rect tile;
	int row = (int)floor(tile_num / tileset_width);
	int col = tile_num % tileset_width;	
	tile.x = col * tile_width;
	tile.y = row * tile_height;
	tile.w = tile_width;
	tile.h = tile_height;
	return tile;
}

void Tileset::load(SDL_Renderer* renderer, std::string path) {
	texture = load_texture(renderer, path);
}

void Tileset::draw_to(SDL_Renderer* renderer, int tile_num, int x, int y) {
	SDL_Rect src_r = get_tile(tile_num);
	SDL_Rect dest_r;
	dest_r.x = x;
	dest_r.y = y;
	dest_r.w = tile_width;
	dest_r.h = tile_height;
	SDL_RenderCopy(renderer, texture, &src_r, &dest_r);
};