#include "game_assets.h"



void TileMap::generate_collision_boxes() {
	int x;
	int y;
	for (x = 0; x < 21; x++) {
		for (y = 0; y < 15; y++) {
			if (dense0[y][x] == '#') {
				collision_boxes.push_back({ x + origin.x,y + origin.y,1,1 });
			}
			if (dense0[y][x] == '>') {
				collision_boxes.push_back({ x + origin.x+0.5f,y + origin.y,0.5f,1 });
			}
			if (dense0[y][x] == '<') {
				collision_boxes.push_back({ x + origin.x,y + origin.y,0.5f,1 });
			}
			if (dense0[y][x] == '_') {
				collision_boxes.push_back({ x + origin.x,y + origin.y+0.3f,1,0.7f});
			}
			if (dense0[y][x] == '^') {
				collision_boxes.push_back({ x + origin.x,y + origin.y,1,0.5f });
			}
			if (dense0[y][x] == '.') {
				collision_boxes.push_back({ x + origin.x+ 0.5f, y + origin.y+0.5f ,0.5f,0.5f });
			}
			if (dense0[y][x] == ',') {
				collision_boxes.push_back({ x + origin.x, y + origin.y + 0.5f ,0.5f,0.5f });
			}
			if (dense0[y][x] == '`') {
				collision_boxes.push_back({ x + origin.x, y + origin.y ,0.5f,0.5f });
			}
			if (dense0[y][x] == '\'') {
				collision_boxes.push_back({ x + origin.x+0.5f, y + origin.y ,0.5f,0.5f });
			}
		}
	}
}
// TODO:
//   Need to unload textures when program closes
void GameAssets::load_assets(SDL_Renderer* renderer) {
	ts_world.load(renderer, "res/terrain.png");
	
	world.generate_collision_boxes();
	ss_playership.load(renderer, "res/stg01_obj/player_plane.png", 64, 64);
	int frames0[] = { 0,1,2 };
	ss_playership.add_animation("FLIP_UP", 3, frames0, 10, false, true);
	int frames1[] = { 0,3,4 };
	ss_playership.add_animation("FLIP_DOWN", 3, frames1, 10, false, true);
	/*
	int frames0[] = { 0,1,2,3 };
	ss_squirrel.add_animation("WALK_DOWN", 4, frames0, 6, true);
	int frames1[] = { 26,27,28,29 };
	ss_squirrel.add_animation("WALK_RIGHT", 4, frames1, 6, true);
	int frames2[] = { 52,53,54,55 };
	ss_squirrel.add_animation("WALK_UP", 4, frames2, 6, true);
	int frames3[] = { 78,79,80,81
	};
	ss_squirrel.add_animation("WALK_LEFT", 4, frames3, 6, true);
	*/

	spritesets["SHOT1"] = CustomSpriteset(renderer, "res/stg01_obj/shot.png", 32, 32);
	spritesets["BOT1"] = CustomSpriteset(renderer, "res/stg01_obj/enemy_zako1.png", 32, 32);
	int frames3[] = {17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
	spritesets["BOT1"].add_animation("IDLE", 15, frames3, 6, true, false);

	backgrounds["NEBULA"] = Background(renderer, "res/nebula_blue.png", 0.95f, true, { 0,0 });
	backgrounds["STARS1"] = Background(renderer, "res/stars_small_1.png", 0.7f, true, { 0,0 });
	backgrounds["STARS2"] = Background(renderer, "res/stars_big_1.png", 0.4f, true, { 0,0 });

	backgrounds["LEVEL1"] = Background(renderer, "res/bg_ship.png", 0.0f,false, { 0,0 });

	level1.init();
}