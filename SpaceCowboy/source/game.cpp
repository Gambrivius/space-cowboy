#include <SDL_ttf.h>
#include <SDL.h>
#include <stdio.h>
#include <vector>
#include "game.h"
#include "game_math.h"
#include "camera.h"
#include "gfx_util.h"
#include "scripting.h"
#include "projectile.h"

bool Game::init() {
	bool success = true;
	
	lua.SetGame(this);

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Check for joysticks
		if (SDL_NumJoysticks() < 1)
		{
			printf("Warning: No joysticks connected!\n");
		}
		else
		{
			//Load joystick
			gamepad = SDL_JoystickOpen(0);
			if (gamepad == NULL)
			{
				printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
			}
		}
		//Create window
		window = SDL_CreateWindow("Space Cowboy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Get window surface
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		}
	}


	if (TTF_Init() == -1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}
	// create camera object
	camera = { SCREEN_WIDTH / SCALE, SCREEN_HEIGHT / SCALE };
	camera.location.x = 320;
	camera.location.y = 240;
	
	player.location.x = 320;
	player.location.y = 240;
	distance_traveled = 0;
	camera_follow_path = false;
	Polygon player_collision_box;
	player_collision_box.vertices.push_back({ -27, -11 });
	player_collision_box.vertices.push_back({ 28, 0 });
	player_collision_box.vertices.push_back({ -27, 11 });
	player.set_collision_polygon(player_collision_box);

	render_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
	hud_font = TTF_OpenFont("res/dpcomic.ttf", 24);
	if (!hud_font) {
		printf("Unable to load font! SDL_ttf Error: %s\n", TTF_GetError());
	}


	
	return success;
};

bool Game::load_media(SDL_Renderer* renderer) {
	game_assets.load_assets(renderer);

	//current_level = &game_assets.level1;
	lua.LoadLevel("scripts/level1.lua");
	player.level = current_level;
	player.spriteset = &game_assets.ss_playership;
	player.animation_tracker.animation = NULL;
	player.animation_tracker.reset();
	gun = new gunLaser();

	camera.location = player.location;
	current_level->entities.push_back(&player);
	lua.CallFunction("StartGame");

	return true;
};

void Game::close() {
	//Destroy window
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	window = NULL;
	delete(current_level);
	SDL_JoystickClose(gamepad);
	gamepad = NULL;
	IMG_Quit();
	TTF_CloseFont(hud_font);
	TTF_Quit();
	//Quit SDL subsystems
	SDL_Quit();
};


void Game::handle_events() {
	Vector2 input_vector;
	input_vector.x = 0;
	input_vector.y = 0;
	
 	while (SDL_PollEvent(&event) != 0)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			exit_request = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			handle_mouse_press(event.button);
			break;
		case SDL_MOUSEBUTTONUP:
			handle_mouse_release(event.button);
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
				case SDLK_ESCAPE:
					exit_request = true;
					break;
				case SDLK_b:
					printf("Creating bezier curve\n");
					current_level->main_path->BuildCurve({ camera.location.x, camera.location.y });
					break;
				case SDLK_l:
					printf("Creating line\n");
					current_level->main_path->BuildLine({ camera.location.x, camera.location.y });
					break;
				case SDLK_c:
					printf("Adding control point to bezier\n");
					current_level->main_path->AddBezierPoint({ camera.location.x, camera.location.y });
					break;
				case SDLK_p:
					camera_follow_path = true;
					distance_traveled = 0;
					player.location = camera.location;
					break;
				case SDLK_o:
					current_level->main_path->PrettyPrint();
					break;
				case SDLK_v:
					camera.draw_path = !camera.draw_path;
					break;

			}
			break;
		default:
			break;
		}
	}
	const Uint8* state = SDL_GetKeyboardState(NULL);

	if (SDL_JoystickGetHat(gamepad, 0) & SDL_HAT_UP)
		input_vector.y -= 1;
	if (SDL_JoystickGetHat(gamepad, 0) & SDL_HAT_DOWN)
		input_vector.y += 1;
	if (SDL_JoystickGetHat(gamepad, 0) & SDL_HAT_LEFT)
		input_vector.x -= 1;
	if (SDL_JoystickGetHat(gamepad, 0) & SDL_HAT_RIGHT)
		input_vector.x += 1;
	if (input_vector.x == 0 && input_vector.y == 0) {
		if (state[SDL_SCANCODE_UP]) input_vector.y -= 1;
		if (state[SDL_SCANCODE_DOWN]) input_vector.y += 1;
		if (state[SDL_SCANCODE_LEFT]) input_vector.x -= 1;
		if (state[SDL_SCANCODE_RIGHT]) input_vector.x += 1;
	}

	if (state[SDL_SCANCODE_SPACE]) gun->fireIfReady(player.location, current_level);
	if (SDL_JoystickGetButton(gamepad, 0)) gun->fireIfReady(player.location, current_level);
	
	input_vector.normalize();
	input_vector.scale(delta_time*player.acceleration);
	player.velocity.add_vector(input_vector);
	player.velocity.clamp(player.max_velocity);
	
	Vector2 move;
	move.x = player.velocity.x;
	move.y = player.velocity.y;
	move.scale(delta_time);
    //move.add_vector(player.location);
	Vector2 camera_follow_velocity = { 0,0 };
	Vector2 old_location = camera.location;
	current_level->UpdateMovementControllers(delta_time);
	camera_follow_velocity = camera.location - old_location;
	
	player.move_and_glide(move + camera_follow_velocity, &player.velocity);
	

	Vector2 zero_vector;
	zero_vector.x = 0;
	zero_vector.y = 0;
	player.velocity.move_towards(zero_vector, delta_time*player.resistance);

	if (player.velocity.x == 0 and player.velocity.y == 0) player.cancel_animation();
	else {
		std::string new_anim = "";

		Vector2 cardinal_vector = player.velocity.get_closest_cardinal();

		//if (cardinal_vector.x == 1 && cardinal_vector.y == 0) new_anim = "WALK_RIGHT";
		//if (cardinal_vector.x == -1 && cardinal_vector.y == 0) new_anim = "WALK_LEFT";
		if (player.velocity.y > 0) new_anim = "FLIP_DOWN";
		else if (player.velocity.y < 0) new_anim = "FLIP_UP";
		else player.cancel_animation();
		player.set_animation(new_anim);
		
	}

}

void Game::draw() {
	camera.draw(renderer, current_level, &game_assets);


	
}

void Game::draw_hud() {
	

	// this is the color in rgb format,
	// maxing out all would give you the color white,
	// and it will be your text's color
	
	

	// as TTF_RenderText_Solid could only be used on
	// SDL_Surface then you have to create the surface first
	char str[32];
	sprintf_s(str, "FPS: %0.f", fps);
	int mWidth=0;
	int mHeight=0;
	SDL_Texture* Message = NULL;
	if (hud_font == NULL) {
		return;
	}
	SDL_Surface* surfaceMessage = TTF_RenderText_Solid(hud_font, str, White);
	if (surfaceMessage == NULL)
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}
	else
	{
		//Create texture from surface pixels

		Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
		if (Message == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = surfaceMessage->w;
			mHeight = surfaceMessage->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(surfaceMessage);
	}
	// now you can convert it into a texture
	SDL_Rect Message_rect; //create a rect
	Message_rect.x = 0;  //controls the rect's x coordinate 
	Message_rect.y = 0; // controls the rect's y coordinte
	Message_rect.w = mWidth; // controls the width of the rect
	Message_rect.h = mHeight; // controls the height of the rect

	SDL_RenderCopy(renderer, Message, NULL, &Message_rect);

	// Don't forget to free your surface and texture
	SDL_DestroyTexture(Message);
}
void Game::start() {
	SDL_Rect from_rect;
	SDL_Rect to_rect;
	from_rect.x = 0;
	from_rect.y = 0;
	from_rect.w = SCREEN_WIDTH / SCALE;
	from_rect.h = SCREEN_HEIGHT / SCALE;

	to_rect.x = 0;
	to_rect.y = 0;
	to_rect.w = SCREEN_WIDTH;
	to_rect.h = SCREEN_HEIGHT;

	while (!exit_request) {
		start_time = end_time;
		
		handle_events();
		
		// draw to temporary render_target texture
		SDL_SetRenderTarget(renderer, render_target);
		SDL_RenderClear(renderer);
		//test_draw();
		draw();
		draw_hud();
		
		SDL_SetRenderTarget(renderer, NULL);
		SDL_RenderClear(renderer);

		// copy the render texture to the renderer's buffer with a scale of 2
		SDL_RenderCopyEx(renderer, render_target, &from_rect, &to_rect, 0, NULL, SDL_FLIP_NONE);

		// swap render buffers
		SDL_RenderPresent(renderer);
		
		// update delta time and delay if necessary to achieve desired frame rate
		update_delta_time();
		Uint32 delay = 1000 / TARGET_FPS;
		if (delay > delta_time)
		{
			SDL_Delay(delay - (Uint32)delta_time);
		}
		update_delta_time();
		
	}
};

void Game::update_delta_time()
{
	end_time = SDL_GetPerformanceCounter();
	delta_time = ((end_time - start_time) * 1000 / (double)SDL_GetPerformanceFrequency());
	fps = 1000 / delta_time;
}

void Game::test_draw() {
	//camera.draw(renderer, current_map, &game_assets);
	
	/*
	SDL_RenderDrawLine(renderer, (int)curve_test.p0.x, (int)curve_test.p0.y, (int)curve_test.p1.x, (int)curve_test.p1.y);
	
	SDL_RenderDrawLine(renderer, (int)curve_test.p2.x, (int)curve_test.p2.y, (int)curve_test.p3.x, (int)curve_test.p3.y);

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	DrawCircle(renderer, (int)curve_test.p0.x, (int)curve_test.p0.y, 5);
	DrawCircle(renderer, (int)curve_test.p1.x, (int)curve_test.p1.y, 5);
	DrawCircle(renderer, (int)curve_test.p2.x, (int)curve_test.p2.y, 5);
	DrawCircle(renderer, (int)curve_test.p3.x, (int)curve_test.p3.y, 5);
	*/
	
}

void Game::draw_path() {
	
}

void Game::handle_mouse_press(SDL_MouseButtonEvent& b)
{
	int x, y;
	Vector2 point;
	SDL_GetMouseState(&x, &y);
	point.x = x/SCALE;
	point.y = y/SCALE;

	point.x = (point.x + camera.location.x - camera.width / 2);
	point.y = (point.y + camera.location.y - camera.height / 2);

	selected_vector = NULL;
	
	PathComponent* p = current_level->main_path->GetRoot();
	while (p)
	{
		NBezier* curve = dynamic_cast<NBezier*>(p);
		if (curve)
		{
			for (int n = 0; n < (int)curve->points.size(); n++)
			{
				if (point.get_distance(*curve->points[n]) <= 5)
					selected_vector = curve->points[n];
			}
	
		}
		Line* line = dynamic_cast<Line*>(p);
		if (line)
		{
			if (point.get_distance(*line->start) <= 5)
				selected_vector = line->start;
			if (point.get_distance(*line->end) <= 5)
				selected_vector = line->end;
		}
		p = p->GetNextComponent();
	}

}
void Game::handle_mouse_release(SDL_MouseButtonEvent& b)
{
	int x, y;

	SDL_GetMouseState(&x, &y);
	x = x / SCALE + camera.location.x - camera.width / 2;
	y = y / SCALE + camera.location.y - camera.height / 2;
	if (selected_vector)
	{
		selected_vector->x = (float)x;
		selected_vector->y = (float)y;
		selected_vector = NULL;
		current_level->main_path->UpdateLengths();
	}
}

