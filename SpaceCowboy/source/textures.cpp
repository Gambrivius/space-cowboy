#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include "textures.h"

SDL_Texture* load_texture(SDL_Renderer* renderer, std::string path)
{
	SDL_Texture* loaded_texture = NULL;

	//Load image at specified path
	SDL_Surface* loaded_surface = IMG_Load(path.c_str());
	if (loaded_surface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Convert surface to texture
		loaded_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
		if (loaded_texture == NULL)
		{
			printf("Unable to load texture %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loaded_surface);
	}
	
	return loaded_texture;
}