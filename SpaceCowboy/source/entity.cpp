#include <string>
#include "entity.h"
#include "game_assets.h"
void Entity::move_and_glide(Vector2 v, Vector2* new_velocity) {
	Vector2 try_pos;
	try_pos.x = location.x + v.x;
	try_pos.y = location.y;

	if (!move(try_pos)) {
		new_velocity->x = 0;
	}
	try_pos.x = location.x;
	try_pos.y = location.y + v.y;
	if (!move(try_pos)) {

		new_velocity->y = 0;
	}
	
}
bool Entity::move(Vector2 new_pos) {
	// check collisions in area
	if (!would_collide_against(&(level->collision_triangles), new_pos))
	{
		set_position(new_pos);
		return true;
	}
	return false;
	
}

void Entity::set_animation(std::string id) {
	if (id != animation_id) {
		Animation* anim = spriteset->get_animation(id);
		if (anim)
		{
			animation_id = id;
			animation_tracker.animation = anim;
			animation_tracker.reset();
		}
	}
}

void Entity::cancel_animation() {
	if (animation_tracker.animation != NULL) {
		animation_tracker.reset();
		animation_tracker.animation = NULL;
		animation_id = "";
		sprite_num = 0;
	}
}

void Entity::set_collision_polygon(Polygon p)
{
	collision_polygon = p;
	collision_triangles = collision_polygon.Triangulate();
}

bool Entity::is_colliding_against(std::vector<Triangle>* triangles)
{
	for (Triangle triangle1 : get_translated_triangles(location))
	{
		for (Triangle triangle2 : *triangles)
		{
			if (triangle_overlap(triangle1, triangle2))
				return true;
		}
	}
	return false;
}

bool Entity::would_collide_against(std::vector<Triangle>* triangles, Vector2 new_location)
{
	for (Triangle triangle1 : get_translated_triangles(new_location))
	{
		for (Triangle triangle2 : *triangles)
		{
			if (triangle_overlap(triangle1, triangle2))
				return true;
		}
	}
	return false;
}

std::vector<Triangle> Entity::get_translated_triangles(Vector2 offset) {
	std::vector<Triangle> triangles;
	for (Triangle t : collision_triangles)
	{
		Triangle t2 = t;
		t2.va = t2.va+ offset;
		t2.vb = t2.vb+ offset;
		t2.vc = t2.vc+ offset;
		triangles.emplace_back(t2);
	}
	return triangles;
}