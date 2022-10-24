#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "game_assets.h"
#include "tileset.h"
#include "spriteset.h"
#include "game_math.h"
#include "entity.h"
#include "background.h"
#include "level.h"


class TileMap {
public:
	Vector2 origin;
	std::vector<Entity*> entities;
	std::vector<Entity*>::iterator entity_it = entities.begin();
	std::vector<Rect> collision_boxes;
	int w = 21;
	int h = 15;
	void generate_collision_boxes();
	char layer0[15][21] = {
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    "
	};
	char layer1[15][21] = {
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    "
	};
	char dense0[15][21] = {
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    ",
						"                    "
	};
	TileMap() {
		origin.x = 0;
		origin.y = 0;
	}
};

class GameAssets {
public:
	Tileset ts_world;
	Spriteset ss_playership;
	TileMap world;
	Level1 level1;

	/*
	Background bg_ship;
	Background bg_space;
	Background bg_stars1;
	Background bg_stars2;
	*/
	std::unordered_map<std::string, Background> backgrounds;
	std::unordered_map<std::string, Spriteset> spritesets;

	void load_assets(SDL_Renderer* renderer);
};

