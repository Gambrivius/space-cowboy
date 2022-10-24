#pragma once
#include <SDL.h>
#include "game_assets.h"
#include "game_math.h"
#include "level.h"


class Camera: public Dynamic {
public:
	
	int width = 640;
	int height = 480;
	Camera (int w, int h) : width (w), height(h)  {
		location.x = 0;
		location.y = 0;
	}
	Camera()
	{
		Camera(100, 100);
	}
	void draw(SDL_Renderer* renderer, Level* level, GameAssets* assets);

	bool draw_path = false;
	Vector2 offset_vector();
};
