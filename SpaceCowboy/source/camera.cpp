#include "camera.h"
#include "game_assets.h"
#include "game_math.h"
#include "gfx_util.h"
#include "projectile.h"
#include <stdio.h>
#include <vector>

void Camera::draw(SDL_Renderer* renderer, Level* level, GameAssets* assets) {
	int xpos = 0;
	int ypos = 0;
	Vector2 scan_ul;		// upper left boundary to scan from
	Vector2 scan_br;		// bottom right boundary to scan to
	scan_ul.x = Dynamic::location.x - width / 2 - 32;
	scan_ul.y = location.y - height / 2 - 32;
	scan_br.x = location.x + width / 2 + 32;
	scan_br.y = location.y + height / 2 + 32;
	
	for (std::string bg : level->backgrounds)
	{
		assets->backgrounds[bg].draw_to(renderer, location, width, height);
	}

	// TODO: Later do this with quad trees instead of iterating over every object.
	for (level->entity_it = level->entities.begin(); level->entity_it != level->entities.end(); ++level->entity_it) {
		Vector2 loc = (*level->entity_it)->location + (*level->entity_it)->v_offset;
		
		if (loc.x >= scan_ul.x && loc.x <= scan_br.x)
		{
			if (loc.y >= scan_ul.y && loc.y <= scan_br.y)
			{
				float render_x = (loc.x - location.x + width / 2);
				float render_y = (loc.y - location.y + height / 2);
				if ((*level->entity_it)->animation_tracker.animation != NULL) {
					(*level->entity_it)->animation_tracker.tick();
					(*level->entity_it)->sprite_num = (*level->entity_it)->animation_tracker.get_sprite_num();
				}
				(*level->entity_it)->spriteset->draw_to(renderer, (*level->entity_it)->sprite_num, (int)render_x, (int)render_y);
			}
		}
	}

	for (auto& p : level->projectiles) {
		//Vector2 loc = (*level->entity_it)->location + (*level->entity_it)->v_offset;

		if (p->location.x >= scan_ul.x && p->location.x <= scan_br.x)
		{
			if (p->location.y >= scan_ul.y && p->location.y <= scan_br.y)
			{
				float render_x = (p->location.x - location.x + width / 2);
				float render_y = (p->location.y - location.y + height / 2);
				if (p->animation_tracker.animation != NULL) {
					p->animation_tracker.tick();
					p->sprite_num = p->animation_tracker.get_sprite_num();
				}
				p->spriteset->draw_to(renderer, p->sprite_num, (int)render_x, (int)render_y);
			}
		}
	}

	// draw paths
	if (draw_path)
	{
		for (Path* path : level->paths)
		{
			PathComponent* p = path->GetRoot();
			while (p)
			{
				NBezier* curve = dynamic_cast<NBezier*>(p);
				if (curve)
				{
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					for (int n = 0; n < (int)curve->points.size(); n++)
					{
						int render_x = (curve->points[n]->x - location.x + width / 2);
						int render_y = (curve->points[n]->y - location.y + height / 2);
						DrawCircle(renderer, render_x, render_y, 5);
					}
					Vector2 p0 = curve->GetPoint(0);
					Vector2 p1;
					SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
					for (float i = 0; i <= 1; i += 0.05f)
					{
						p1 = curve->GetPoint(i);
						int r_x0 = (p0.x - location.x + width / 2);
						int r_y0 = (p0.y - location.y + height / 2);
						int r_x1 = (p1.x - location.x + width / 2);
						int r_y1 = (p1.y - location.y + height / 2);
						SDL_RenderDrawLine(renderer, r_x0, r_y0, r_x1, r_y1);
						p0 = p1;
					}
					p1 = curve->GetPoint(1);
					int r_x0 = (p0.x - location.x + width / 2);
					int r_y0 = (p0.y - location.y + height / 2);
					int r_x1 = (p1.x - location.x + width / 2);
					int r_y1 = (p1.y - location.y + height / 2);
					SDL_RenderDrawLine(renderer, r_x0, r_y0, r_x1, r_y1);
				}
				Line* line = dynamic_cast<Line*>(p);
				if (line)
				{
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					int r_x0 = (line->start->x - location.x + width / 2);
					int r_y0 = (line->start->y - location.y + height / 2);
					int r_x1 = (line->end->x - location.x + width / 2);
					int r_y1 = (line->end->y - location.y + height / 2);

					DrawCircle(renderer, r_x0, r_y0, 5);
					DrawCircle(renderer, r_x1, r_y1, 5);
					SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
					SDL_RenderDrawLine(renderer, r_x0, r_y0, r_x1, r_y1);
				}
				p = p->GetNextComponent();
			}
		}
		
	}

	
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	
}
Vector2 Camera::offset_vector()
{
	Vector2 offset = location + Vector2(width / 2, height / 2);
	return offset;
}