#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "camera.h"
#include "game_assets.h"
#include "game_math.h"
#include "entity.h"
#include "level.h"
#include "scripting.h"
#include "projectile.h"

#define SCALE           3
#define SCREEN_WIDTH    568*SCALE
#define SCREEN_HEIGHT   320*SCALE
#define BASE_TILE_SIZE  32
#define TARGET_FPS      120
const int JOYSTICK_DEAD_ZONE = 8000;



class Game
{
public:
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Joystick* gamepad = NULL;
    SDL_Texture* render_target = NULL;
    Level* current_level = NULL;
    TTF_Font* hud_font = NULL;
    SDL_Color White = { 255, 255, 255 };
    Camera camera;
    Entity player;
    LuaEngine lua;
    double delta_time = 0;
    double fps = 0;

    bool exit_request = false;
    SDL_Event event;
    GameAssets game_assets;

    Uint64 start_time = 0;
    Uint64 end_time = 0;
    Gun* gun;

    bool init();
    bool load_media(SDL_Renderer* renderer);
    void handle_events();
    void draw();
    void draw_hud();
    
    void start();
    void close();

    void update_delta_time();

    void test_draw();
    void draw_path();
    void handle_mouse_press(SDL_MouseButtonEvent& b);
    void handle_mouse_release(SDL_MouseButtonEvent& b);
    Vector2* selected_vector;
    float distance_traveled =0;
    bool camera_follow_path = false;

    Polygon test_polygon;

    Level* new_level()
    {
        if (current_level) delete current_level;
        current_level = new CustomLevel();
        return current_level;
    }
    void delete_level()
    {
        delete current_level;
    }
};

