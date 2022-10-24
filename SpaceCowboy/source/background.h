#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <vector>
#include "game_math.h"
#include "textures.h"

struct RenderBox {
	SDL_Rect from = { 0,0,0,0 };
	SDL_Rect to = { 0, 0, 0, 0 };
	RenderBox(float fx, float fy, float tx, float ty, float w, float h) {
		from.x = (int)fx;
		from.y = (int)fy;
		from.w = (int)w;
		from.h = (int)h;
		to.x = (int)tx;
		to.y = (int)ty;
		to.w = from.w;
		to.h = from.h;
	}
	RenderBox() {}
};
class Background
{
public:
	SDL_Texture* texture = NULL;
	//std::string filepath;
	Vector2 origin = { 0,0 };
	int t_width;
	int t_height;
	float parallax_effect = 0.0f;
	bool wrap_around = true;
	Background()
	{

	}
	Background(SDL_Renderer* renderer, std::string filepath, float ParallaxEffect, bool WrapAround, Vector2 Origin)
		: parallax_effect (ParallaxEffect), wrap_around (WrapAround), origin (Origin)
	{
		load(renderer, filepath);

	}
	Background(SDL_Renderer* renderer, std::string filepath) {
		// default constructor, requires at least a file path and renderer.

		Background(renderer, filepath, 0.0f, true, { 0,0 });
	}

	void wrap_to(SDL_Renderer* renderer, Vector2 location, int width, int height)
	{
		// algorithm for wrapping background textures around the screen.

		// first, normalize the center vector based on tile size
		// and texture dimensions
		Vector2 center = (1-parallax_effect) * location - (1 - parallax_effect)  * origin;
		while (center.x > t_width) center.x -= t_width;
		while (center.x < 0) center.x += t_width;
		while (center.y > t_height) center.y -= t_height;
		while (center.y < 0) center.y += t_height;

		// next, build a vector of rectangles
		std::vector<RenderBox> render_boxes;
		render_boxes.emplace_back(center.x-width/2, center.y-height/2, 0,0, width, height);

		bool check_bounds = true;
		while (check_bounds)
		{
			check_bounds = false;
			std::vector<RenderBox>::iterator render_it = render_boxes.begin();
			for (render_it = render_boxes.begin(); render_it != render_boxes.end(); ++render_it)
			{
				if (render_it->from.x < 0 )
				{
					check_bounds = true;
					// split horizontally
					RenderBox left, right;
					int x_offset = 0 - render_it->from.x;
					left.from.x = t_width - x_offset;
					left.from.y = render_it->from.y;
					left.from.w = x_offset;
					left.from.h = render_it->from.h;

					left.to.x = 0;
					left.to.y = render_it->to.y;
					left.to.w = x_offset;
					left.to.h = left.from.h;

					right.from.x = 0;
					right.from.y = render_it->from.y;
					right.from.w = width - x_offset;
					right.from.h = render_it->from.h;
					
					right.to.x = x_offset;
					right.to.y = render_it->to.y;
					right.to.w = width - x_offset;
					right.to.h = render_it->to.h;
					render_boxes.erase(render_it);
					render_boxes.push_back(left);
					render_boxes.push_back(right);
					render_it = render_boxes.begin();
				}
				if (render_it->from.x + render_it->from.w > t_width)
				{
					check_bounds = true;
					// split horizontally
					RenderBox left, right;
					int x_offset = t_width-render_it->from.x;
					left.from.x = render_it->from.x;
					left.from.y = render_it->from.y;
					left.from.w = x_offset;
					left.from.h = render_it->from.h;

					left.to.x = render_it->to.x;
					left.to.y = render_it->to.y;
					left.to.w = x_offset;
					left.to.h = left.from.h;
					
					right.from.x = 0;
					right.from.y = render_it->from.y;
					right.from.w = width - x_offset;
					right.from.h = render_it->from.h;

					right.to.x = x_offset;
					right.to.y = render_it->to.y;
					right.to.w = width - x_offset;
					right.to.h = render_it->to.h;
					render_boxes.erase(render_it);
					render_boxes.push_back(left);
					render_boxes.push_back(right);
					render_it = render_boxes.begin();
				}
				if (render_it->from.y < 0)
				{
					check_bounds = true;
					// split vertically
					RenderBox top, bottom;
					int y_offset = 0 - render_it->from.y;
					top.from.x = render_it->from.x;
					top.from.y = t_height - y_offset;
					top.from.w = render_it->from.w;
					top.from.h = y_offset;

					top.to.x = render_it->to.x;
					top.to.y = 0;
					top.to.w = top.from.w;
					top.to.h = y_offset;

					bottom.from.x = render_it->from.x;
					bottom.from.y = 0;
					bottom.from.w = render_it->from.w;
					bottom.from.h = height-top.from.h;

					bottom.to.x = render_it->to.x;
					bottom.to.y = top.to.y + top.to.h;
					bottom.to.w = render_it->to.w;
					bottom.to.h = bottom.from.h;
					render_boxes.erase(render_it);
					render_boxes.push_back(top);
					render_boxes.push_back(bottom);
					render_it = render_boxes.begin();
				}
				if (render_it->from.y + render_it->from.h > t_height)
				{
					check_bounds = true;
					// split vertically
					RenderBox top, bottom;
					
					int y_offset = t_height - render_it->from.y;;
					top.from.x = render_it->from.x;
					top.from.y = render_it->from.y;
					top.from.w = render_it->from.w;
					top.from.h = y_offset;

					top.to.x = render_it->to.x;
					top.to.y = render_it->to.y;
					top.to.w = render_it->from.w;
					top.to.h = y_offset;


					bottom.from.x = render_it->from.x;
					bottom.from.y = 0;
					bottom.from.w = render_it->from.w;
					bottom.from.h = height - top.from.h;

					bottom.to.x = render_it->to.x;
					bottom.to.y = top.to.y + top.to.h;
					bottom.to.w = render_it->to.w;
					bottom.to.h = bottom.from.h;
					render_boxes.erase(render_it);
					render_boxes.push_back(top);
					render_boxes.push_back(bottom);
					render_it = render_boxes.begin();
				}

			}
		}


		std::vector<RenderBox>::iterator render_it;
		for (render_it = render_boxes.begin(); render_it != render_boxes.end(); ++render_it)
		{
			SDL_RenderCopy(renderer, texture, &render_it->from, &render_it->to);
		}
		
		
		
				
	}
	void draw_to(SDL_Renderer* renderer, Vector2 location, int width, int height)
	{
		if (wrap_around) wrap_to (renderer, location, width, height);
		else
		{
			Vector2 center = (1 - parallax_effect)  * location - (1 - parallax_effect)  * origin;
			clip_to(renderer, center, width, height);
		}
		
	}

	void clip_to(SDL_Renderer* renderer, Vector2 center, int width, int height)
	{
		// determine viewable area of background and clip the image
		SDL_Rect src_r;
		src_r.x = center.x - width / 2;
		src_r.y = center.y - height / 2;
		src_r.w = width;
		src_r.h = height;



		SDL_Rect dest_r;
		dest_r.x = 0;
		dest_r.y = 0;
		dest_r.w = width;
		dest_r.h = height;
		// screen clipping calculations
		if (src_r.y < 0)
		{
			int offset = 0 - src_r.y;
			src_r.h -= offset;
			dest_r.h -= offset;
			src_r.y = 0;
			dest_r.y = offset;
		}
		if (src_r.y + src_r.h > t_height)
		{
			int offset = src_r.y + src_r.h - t_height;
			src_r.h -= offset;
			dest_r.h -= offset;
		}
		if (src_r.x < 0)
		{
			int offset = 0 - src_r.x;
			src_r.w -= offset;
			dest_r.w -= offset;
			src_r.x = 0;
			dest_r.x = offset;
		}
		if (src_r.x + src_r.w > t_width)
		{
			int offset = src_r.x + src_r.w - t_width;
			src_r.w -= offset;
			dest_r.w -= offset;
		}
		SDL_RenderCopy(renderer, texture, &src_r, &dest_r);
	}

	void load(SDL_Renderer* renderer, std::string filepath)
	{
		texture = load_texture(renderer, filepath);
		SDL_QueryTexture(texture, NULL, NULL, &t_width, &t_height);
	}


};