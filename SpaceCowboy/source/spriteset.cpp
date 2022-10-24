#include <SDL.h>
#include <stdio.h>
#include <string>
#include "spriteset.h"
#include "textures.h"

SDL_Rect Spriteset::get_sprite(int sprite_num) {
	SDL_Rect tile;
	int row = (int)floor(sprite_num / tileset_width);
	int col = sprite_num % tileset_width;
	tile.x = col * tile_width;
	tile.y = row * tile_height;
	tile.w = tile_width;
	tile.h = tile_height;
	return tile;
}

void Spriteset::load(SDL_Renderer* renderer, std::string path, int tileWidth, int tileHeight) {
	texture = load_texture(renderer, path);
	tile_width = tileWidth;
	tile_height = tileHeight;
	SDL_QueryTexture(texture, NULL, NULL, &tileset_width, &tileset_height);
	tileset_width /= tile_width;
	tileset_height /= tile_height;
}

void Spriteset::draw_to(SDL_Renderer* renderer, int sprite_num, int x, int y) {

	// i could pass a shader object here as an argument
	// the sprite would render to an intermediate texture,
	// int SDL_SetTextureColorMod(SDL_Texture * texture,
	//       Uint8 r, Uint8 g, Uint8 b);
	// then in the event update loop, we'd have to modify the shaders values with a lerp
	// and delete the shader once it is finished...
	// it could be a member of the entity class

	SDL_Rect src_r = get_sprite(sprite_num);
	SDL_Rect dest_r;
	dest_r.x = x;
	dest_r.y = y;
	dest_r.w = tile_width;
	dest_r.h = tile_height;
	SDL_RenderCopy(renderer, texture, &src_r, &dest_r);
};

Animation* Spriteset::get_animation(std::string s)
{
	if (animation_map.count(s) > 0)
	{
		return &animation_map[s];
	}
	else return NULL;
}


void Animation::insert_frames(int int_arr[], int size)
{
	int i;
	for (i = 0; i < size; ++i) {
		frames.push_back(int_arr[i]);
	}
}

void Spriteset::add_animation(std::string anim_id, int frames, int frame_array[], int frame_rate, bool repeat, bool hold)
{
	animation_map[anim_id] = Animation(frames, frame_array, frame_rate, repeat, hold);
}