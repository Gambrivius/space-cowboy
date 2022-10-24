#include <stdio.h>
#include "editor.h"

#include <SDL_ttf.h>
#include <SDL.h>
#include <stdio.h>
#include <vector>
#include "editor.h"
#include "../SpaceCowboy/source/game_math.h"
#include "../SpaceCowboy/source/camera.h"
#include "../SpaceCowboy/source/gfx_util.h"
bool Editor::init() {
	bool success = true;



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
		window = SDL_CreateWindow("Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
	camera = new Camera(SCREEN_WIDTH / SCALE, SCREEN_HEIGHT / SCALE);
	camera->location.x = 320;
	camera->location.y = 240;

	player.location.x = 320;
	player.location.y = 240;
	distance_traveled = 0;
	camera_follow_path = false;
	camera->draw_path = true;

	render_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
	hud_font = TTF_OpenFont("res/dpcomic.ttf", 24);
	if (!hud_font) {
		printf("Unable to load font! SDL_ttf Error: %s\n", TTF_GetError());
	}

	return success;
};

bool Editor::load_media(SDL_Renderer* renderer) {
	game_assets.load_assets(renderer);

	current_level = &game_assets.level1;
	player.level = &game_assets.level1;
	player.spriteset = &game_assets.ss_playership;
	player.animation_tracker.animation = NULL;
	player.animation_tracker.reset();
	current_level->entities.push_back(&player);
	selected_path = current_level->main_path;
	return true;
};

void Editor::close() {
	//Destroy window
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	window = NULL;
	delete(camera);
	SDL_JoystickClose(gamepad);
	gamepad = NULL;
	IMG_Quit();
	TTF_CloseFont(hud_font);
	TTF_Quit();
	//Quit SDL subsystems
	SDL_Quit();
};


void Editor::handle_events() {
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
			case SDLK_n:
				printf("Creating new path\n");
				current_level->paths.emplace_back(new Path);
				selected_path = current_level->paths.back();
				selected_path->SetFirstBuildPoint(camera->location);
				break;
			case SDLK_b:
				printf("Creating bezier curve\n");
				if(selected_path) selected_path->BuildCurve({ camera->location.x, camera->location.y });
				break;
			case SDLK_l:
				printf("Creating line\n");
				if (selected_path) selected_path->BuildLine({ camera->location.x, camera->location.y });
				break;
			case SDLK_c:
				printf("Adding control point to bezier\n");
				if (selected_path) selected_path->AddBezierPoint({ camera->location.x, camera->location.y });
				break;
			case SDLK_p:
				if (!camera_follow_path) {
					camera_follow_path = true;
					distance_traveled = 0;
					player.location = camera->location;
				}
				else {
					camera_follow_path = false;
				}
				break;
			case SDLK_o:
				current_level->PrettyPrint();
				printf("%.2f, %.2f", player.location.x, player.location.y);
				break;
			case SDLK_v:
				camera->draw_path = !camera->draw_path;
				break;
			case SDLK_z:
				build_new_polygon();
				break;
			case SDLK_x:
				polygon_insert_vertex();
				break;
			case SDLK_t:
				triangles = selected_polygon->triangulate();
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

	input_vector.normalize();
	input_vector.scale(delta_time * player.acceleration);
	player.velocity.add_vector(input_vector);
	player.velocity.clamp(player.max_velocity);

	Vector2 move;
	move.x = player.velocity.x;
	move.y = player.velocity.y;
	move.scale(delta_time);
	//move.add_vector(player.location);
	Vector2 old_loc = player.location;
	player.move_and_glide(move, &player.velocity);
	Triangle player_triangle = { {-27, -10}, {30, 2}, {-28, 10} };
	player_triangle = player_triangle + player.location;
	

	// set camera follow
	if (!camera_follow_path)
		camera->location.move_towards(player.location, delta_time * 0.6);
	else
	{
		if (current_level->main_path->GetRoot())
		{
			// using fmin of delta time as a work around to really high delta_time
			// on first couple of frames, resulting in travelling to the end of the path instantly
			// I think this will be handled better later by not starting the path until after a few frames
			distance_traveled += fmin(delta_time, 100) * 0.05;
			printf("Distanced traveled = %.2f\n", distance_traveled);
			Vector2 old_location = camera->location;
			camera->location = current_level->main_path->GetPointAtDistance(distance_traveled);
			Vector2 cam_vector = camera->location - old_location;
			player.location = player.location + cam_vector;
			if (distance_traveled > current_level->main_path->length)
			{
				camera_follow_path = false;
			}
		}
		else
		{
			// there is no path to follow
			camera_follow_path = false;
		}
	}

	Vector2 zero_vector;
	zero_vector.x = 0;
	zero_vector.y = 0;
	player.velocity.move_towards(zero_vector, delta_time * player.resistance);

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

void Editor::draw() {
	camera->draw(renderer, current_level, &game_assets);


	Triangle player_triangle = { {-27, -12}, {30, 2}, {-28, 12} };
	player_triangle = player_triangle + player.location;
	Vector2 a = player_triangle.va - camera->location + Vector2(camera->width / 2, camera->height / 2);
	Vector2 b = player_triangle.vb - camera->location + Vector2(camera->width / 2, camera->height / 2);
	Vector2 c = player_triangle.vc - camera->location + Vector2(camera->width / 2, camera->height / 2);
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 200);
	SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
	SDL_RenderDrawLine(renderer, b.x, b.y, c.x, c.y);
	SDL_RenderDrawLine(renderer, c.x, c.y, a.x, a.y);
	
	for (Polygon p : current_level->collision_polygons)
	{
		Vector2 p0 = p.vertices[0]-camera->location+Vector2(camera->width/2, camera->height/2);
		Vector2 p1;
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		for (int i = 1; i < p.vertices.size(); i += 1)
		{
			p1 = p.vertices[i] - camera->location + Vector2(camera->width / 2, camera->height / 2);
			SDL_RenderDrawLine(renderer, p0.x, p0.y, p1.x, p1.y);
			p0 = p1;
		}
		p1 = p.vertices[0] - camera->location + Vector2(camera->width / 2, camera->height / 2);
		SDL_RenderDrawLine(renderer, p0.x, p0.y, p1.x, p1.y);

		SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
		for (int n = 0; n <  p.vertices.size(); n++)
		{
			Vector2 r = p.vertices[n] - camera->location + Vector2(camera->width / 2, camera->height / 2);
			DrawCircle(renderer, r.x, r.y, 5);
		}
	}

	// draw triangles

	for (Triangle triangle : current_level->collision_triangles)
	{
		if (triangle_overlap(player_triangle, triangle))
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 200);
		else
			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 200);
		Vector2 a = triangle.va - camera->location + Vector2(camera->width / 2, camera->height / 2);
		Vector2 b = triangle.vb - camera->location + Vector2(camera->width / 2, camera->height / 2);
		Vector2 c = triangle.vc - camera->location + Vector2(camera->width / 2, camera->height / 2);
		SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
		SDL_RenderDrawLine(renderer, b.x, b.y, c.x, c.y);
		SDL_RenderDrawLine(renderer, c.x, c.y, a.x, a.y);
	}

}

void Editor::draw_hud() {


	// this is the color in rgb format,
	// maxing out all would give you the color white,
	// and it will be your text's color



	// as TTF_RenderText_Solid could only be used on
	// SDL_Surface then you have to create the surface first
	char str[32];
	sprintf_s(str, "FPS: %0.f", fps);
	int mWidth = 0;
	int mHeight = 0;
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
void Editor::start() {
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

void Editor::update_delta_time()
{
	end_time = SDL_GetPerformanceCounter();
	delta_time = ((end_time - start_time) * 1000 / (double)SDL_GetPerformanceFrequency());
	fps = 1000 / delta_time;
}

void Editor::test_draw() {
	//camera->draw(renderer, current_map, &game_assets);

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

void Editor::draw_path() {

}

void Editor::handle_mouse_press(SDL_MouseButtonEvent& b)
{
	int x, y;
	Vector2 point;
	SDL_GetMouseState(&x, &y);
	point.x = x / SCALE;
	point.y = y / SCALE;

	point.x = (point.x + camera->location.x - camera->width / 2);
	point.y = (point.y + camera->location.y - camera->height / 2);

	selected_vector = NULL;
	for (Path* path : current_level->paths)
	{
		PathComponent* p = path->GetRoot();
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
	
	
	for (Polygon& p : current_level->collision_polygons)
	{
		for (Vector2& v : p.vertices)
		{
			if (point.get_distance(v) <= 5)
			{
				selected_vector = &v;
			}
			
		}
	}

}
void Editor::handle_mouse_release(SDL_MouseButtonEvent& b)
{
	int x, y;

	SDL_GetMouseState(&x, &y);
	x = x / SCALE + camera->location.x - camera->width / 2;
	y = y / SCALE + camera->location.y - camera->height / 2;
	if (selected_vector)
	{
		selected_vector->x = (float)x;
		selected_vector->y = (float)y;
		selected_vector = NULL;
		current_level->CalculatePathLengths();
	}
}

Polygon* Editor::build_new_polygon()
{
	current_level->collision_polygons.emplace_back();
	Polygon* new_gon = &current_level->collision_polygons.back();
	new_gon->vertices.push_back(camera->location);
	new_gon->vertices.push_back(camera->location + Vector2(25, 0));
	selected_polygon = new_gon;
	return new_gon;
}

void Editor::polygon_insert_vertex()
{
	if (selected_polygon)
	{
		selected_polygon->vertices.push_back(camera->location);
	}
}

int main(int argc, char* args[])
{
	Editor editor;

	if (editor.init()) {
		if (editor.load_media(editor.renderer))
		{
			editor.start();
			editor.close();
		}
	}
	return 0;
}
