#include "entity.h"
#include "level.h"
#include "projectile.h"

Entity* Level::SpawnEntity(Spriteset* spriteset, Vector2 location, std::string default_animation)
{
	Entity* new_entity = new Entity();
	new_entity->spriteset = spriteset;
	new_entity->set_animation(default_animation);
	new_entity->location = location;
	entities.emplace_back(new_entity);
	return entities.back();
}

void Level::CheckProjectileCollisions()
{
	//Uint64 time = SDL_GetPerformanceCounter();
	auto projectile = projectiles.begin();
	while (projectile != projectiles.end())
	{
		if ((*projectile)->is_colliding_against(&collision_triangles))
		{
			projectile = projectiles.erase(projectile);
		}
		else ++projectile;
	}

	projectile = projectiles.begin();
	// check projectile collision against enemies
	while (projectile != projectiles.end())
	{
		auto enemy = entities.begin();
		bool collision = false;
		while (enemy != entities.end())
		{
			if ((*projectile)->is_colliding_against(*enemy))
			{
				// delete enemy and projectile
				(*enemy)->hp -= (*projectile)->damage;
				projectile = projectiles.erase(projectile);

				collision = true;
				if ((*enemy)->hp <= 0)
				{
					enemy = entities.erase(enemy);
				}
				break;
			}
			else ++enemy;
		}
		if (!collision) ++projectile;
	}

	//Uint64 ellapsed_time = (SDL_GetPerformanceCounter() - time);
	//std::cout << "Projectiles: " << projectiles.size() << ", Time (s): " << (float)ellapsed_time/SDL_GetPerformanceFrequency() << std::endl;
}

void Level::UpdateMovementControllers(float delta_time)
{
	delta_time = fmin(delta_time, 10);
	for (auto& c : future_movement_controllers)
	{
		movement_controllers.push_back(c);
	}
	future_movement_controllers.clear();

	for (MovementController* m : movement_controllers)
	{
		m->Update(delta_time);

	}
	for (auto& lua_event_handler : lua_event_handlers)
	{
		lua_event_handler->UpdateEvents();
	}
	auto it = movement_controllers.begin();
	while (it != movement_controllers.end())
	{
		if ((*it)->IsComplete())
		{
			Dynamic* d = (*it)->dynamic_obj;
			Projectile* p = dynamic_cast<Projectile*>(d);
			if (p)
			{
				auto p_it = std::find(projectiles.begin(), projectiles.end(), p);
				if (p_it != projectiles.end()) projectiles.erase(p_it);

			}
			it = movement_controllers.erase(it);
		}
		else ++it;
	}

	CheckProjectileCollisions();
}