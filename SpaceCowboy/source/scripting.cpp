#include "game.h"
#include "scripting.h"


int lua_HostFunction(lua_State* L) {
	// fresh stack
	float a = (float)lua_tonumber(L, 1);
	float b = (float)lua_tonumber(L, 2);
	printf("[C++] HostFunction(%.2f,%.2f) called\n", a, b);
	float c = a * b;
	printf("[C++] Pusing %.2f to stack\n", c);
	lua_pushnumber(L, c);
	return 1; // returns 1 value
}

void LuaEngine::SetGame(Game* g) {
	game = g;

	// set globals
	lua_pushlightuserdata(L, &(game->camera));
	lua_setglobal(L, "CAMERA");
	
}

void LuaEngine::LoadLevel(const char* lua_file)
{
	std::cout << "[LUA ENGINE] Loading level file " << lua_file << std::endl;
	int r = luaL_dofile(L, lua_file);
	if (CheckLua(r))
	{
		lua_getglobal(L, "level_data");
		if (lua_istable(L, -1))
		{
			lua_pushstring(L, "name");
			lua_gettable(L, -2);
			std::string s = lua_tostring(L, -1);
			lua_pop(L, -1);
		}
	}
	
	lua_getglobal(L, "BuildLevel");
	if (lua_isfunction(L, -1))
	{
		lua_pushlightuserdata(L, game); // push a pointer to the stack
		if (CheckLua(lua_pcall(L, 1, 1, 0))) // takes 1 argument, returns 1
		{
			//float f = (float)lua_tonumber(L, -1);
			//printf("Lua returned value %.2f to c++", f);
		}
	}
	else {
		log_error("BuildLevel not defined!");
		lua_pop(L, -1);
	}

}

void LuaEngine::CallFunction(std::string function_name)
{
	lua_getglobal(L, function_name.c_str());
	if (lua_isfunction(L, -1))
	{
		lua_pushlightuserdata(L, game); // push a pointer to the stack
		if (CheckLua(lua_pcall(L, 1, 0, 0))) // takes 1 argument, returns 1
		{
			//float f = (float)lua_tonumber(L, -1);
			//printf("Lua returned value %.2f to c++", f);
		}
	}
	else {
		log_error(function_name + " not defined!");
		lua_pop(L, -1);
	}
}
int LuaEngine::wrap_CreateLevel(lua_State* L)
{
	if (lua_gettop(L) != 1)
	{
		luaL_argerror(L, 1, "game pointer");
		log_error("CreateLevel requires 1 argument");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Game* g = static_cast<Game*> (lua_touserdata(L, 1));
		if (g)
		{
			Level* l = g->new_level();
			l->game = g;


			lua_pushlightuserdata(L, l);
			return 1;
		}
		luaL_typeerror(L, 1, "Unknown object type");
	}
	luaL_typeerror(L, 1, "Reference to game object expected");
	return -1;
}
bool LuaEngine::lua_tovector(lua_State* L, int index, Vector2* v)
{
	if (lua_istable(L, index))
	{
		// get a vector
		lua_pushnumber(L, 1); // push index 1 onto the stack
		lua_gettable(L, 2);
		if (lua_isnumber(L, -1))
		{
			v->x = (float)lua_tonumber(L, -1);
		}

		lua_pushnumber(L, 2); // push index 2 onto the stack

		lua_gettable(L, 2);

		if (lua_isnumber(L, -1))
		{
			v->y = (float)lua_tonumber(L, -1);
		}
		//std::cout << "X = " << v.x << ", Y = " << v.y << std::endl;
		/*
		lua_pop(L, -1);
		lua_pop(L, -1);
		lua_pop(L, -1);
		lua_pop(L, -1);*/
		return true;
	}
	luaL_typeerror(L, 2, "Expected vector table");
	return false;
}

int LuaEngine::wrap_CreatePath(lua_State* L)
{
	if (lua_gettop(L) != 2)
	{
		luaL_argerror(L, 2, "level pointer, origin vector");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Level* level = static_cast<Level*> (lua_touserdata(L, 1));
		if (level)
		{
			Vector2 v;
			if (lua_tovector(L, 2, &v))
			{
				level->paths.emplace_back(new Path);
				Path* p = level->paths.back();
				p->SetFirstBuildPoint(v);
				lua_pushlightuserdata(L, p);
				return 1;
			}
		}
		luaL_typeerror(L, 1, "Unknown object type");
		return -1;
	}
	luaL_typeerror(L, 1, "Reference to level object expected");
	return -1;
}

int LuaEngine::wrap_PathBuildLine(lua_State* L)
{
	if (lua_gettop(L) != 2)
	{
		luaL_argerror(L, 2, "path pointer, vector");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Path* path = static_cast<Path*> (lua_touserdata(L, 1));
		if (path)
		{
			Vector2 v;
			if (lua_tovector(L, 2, &v))
			{
				path->BuildLine(v);
				return 0;
			}
		}
		luaL_typeerror(L, 1, "Unknown object type");
		return -1;
	}
	luaL_typeerror(L, 1, "Reference to level object expected");
	return -1;
}

int LuaEngine::wrap_PathBuildCurve(lua_State* L)
{
	if (lua_gettop(L) != 2)
	{
		luaL_argerror(L, 2, "path pointer, vector");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Path* path = static_cast<Path*> (lua_touserdata(L, 1));
		if (path)
		{
			Vector2 v;
			if (lua_tovector(L, 2, &v))
			{
				path->BuildCurve(v);
				return 0;
			}
		}
		luaL_typeerror(L, 1, "Unknown object type");
		return -1;
	}
	luaL_typeerror(L, 1, "Reference to level object expected");
	return -1;
}
int LuaEngine::wrap_PathBuildControlPoint(lua_State* L)
{
	if (lua_gettop(L) != 2)
	{
		luaL_argerror(L, 2, "path pointer, vector");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Path* path = static_cast<Path*> (lua_touserdata(L, 1));
		if (path)
		{
			Vector2 v;
			if (lua_tovector(L, 2, &v))
			{
				path->AddBezierPoint(v);
				return 0;
			}
		}
		luaL_typeerror(L, 1, "Unknown object type");
		return -1;
	}
	luaL_typeerror(L, 1, "Reference to level object expected");
	return -1;
}

int LuaEngine:: wrap_PathGetLength(lua_State* L)
{
	if (lua_gettop(L) != 1)
	{
		luaL_argerror(L, 1, "path pointer");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Path* path = static_cast<Path*> (lua_touserdata(L, 1));
		if (path)
		{
			float distance = path->length;
			lua_pushnumber(L, distance);
			return 1;
		}
		luaL_typeerror(L, 1, "path object");
		return -1;
	}
	luaL_typeerror(L, 1, "path object");
	return -1;
}

int LuaEngine::wrap_LevelAddBackground(lua_State* L)
{
	if (lua_gettop(L) != 2)
	{
		luaL_argerror(L, 2, "level pointer, string");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Level* level = static_cast<Level*> (lua_touserdata(L, 1));
		if (level)
		{
			if (lua_isstring(L, 2))
			{
				std::string bg_string = lua_tostring(L, 2);
				level->backgrounds.push_back(bg_string);
				return 0;
			}
			luaL_typeerror(L, 2, "Expected string");
		}
		luaL_typeerror(L, 1, "Expected level pointer");
		return -1;
	}
	luaL_typeerror(L, 1, "Reference to level object expected");
	return -1;
}

int LuaEngine::wrap_LevelSetMainPath(lua_State* L)
{
	if (lua_gettop(L) != 2)
	{
		luaL_argerror(L, 2, "level pointer, string");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Level* level = static_cast<Level*> (lua_touserdata(L, 1));
		if (level)
		{
			if (lua_isuserdata(L, 2))
			{
				Path* path = static_cast<Path*> (lua_touserdata(L, 2));
				if (path)
				{
					level->main_path = path;
					return 0;
				}
			}
			luaL_typeerror(L, 2, "Expected path pointer");
			return -1;
		}
		luaL_typeerror(L, 1, "Expected level pointer");
		return -1;
	}
	luaL_typeerror(L, 1, "Reference to level object expected");
	return -1;
}


int LuaEngine::wrap_CreateStaticCollisionPolygon(lua_State* L)
{
	if (lua_gettop(L) != 1)
	{
		luaL_argerror(L, 1, "level pointer");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Level* level = static_cast<Level*> (lua_touserdata(L, 1));
		if (level)
		{
			
			level->collision_polygons.emplace_back();
			Polygon* p = &(level->collision_polygons.back());
			lua_pushlightuserdata(L, p);
			return 1;
		}
		luaL_typeerror(L, 1, "Unknown object type");
		return -1;
	}
	luaL_typeerror(L, 1, "Reference to level object expected");
	return -1;
}

int LuaEngine::wrap_EnemyGetCollisionPolygon(lua_State* L)
{
	if (lua_gettop(L) != 1)
	{
		luaL_argerror(L, 1, "dynamic object pointer");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Dynamic* dynamic_obj = static_cast<Dynamic*> (lua_touserdata(L, 1));
		if (dynamic_obj)
		{
			Polygon* p = dynamic_obj->GetCollisionPolygon();
			lua_pushlightuserdata(L, p);
			return 1;
		}
		luaL_typeerror(L, 1, "dynamic object pointer");
		return -1;
	}
	luaL_typeerror(L, 1, "dynamic object pointer");
	return -1;
}

int LuaEngine::wrap_EnemySetLocationOffset(lua_State* L)
{
	if (lua_gettop(L) != 2)
	{
		luaL_argerror(L, 1, "entity pointer, vector");
		return -1;
	}
	Entity* entity;
	Vector2 offset;
	if (lua_isuserdata(L, 1))
	{
		entity = static_cast<Entity*> (lua_touserdata(L, 1));
		if (!entity)
		{
			luaL_typeerror(L, 1, "entity pointer");
			return -1;
		}
	}
	else {
		luaL_typeerror(L, 1, "entity pointer");
		return -1;
	}

	if (!lua_tovector(L, 2, &offset))
	{
		luaL_typeerror(L, 2, "vector");
		return -1;
	}
	if(entity)	entity->v_offset = offset;
	return 0;
}
// enemy = SpawnEnemy(level, "BOT1", {0,0}, "IDLE")

int LuaEngine::wrap_SpawnEnemy(lua_State* L)
{
	if (lua_gettop(L) != 5)
	{
		luaL_argerror(L, 5, "level ptr, sprite, vector, animation, int hp");
		return -1;
	}
	Level* level;
	std::string sprite_id;
	Vector2 loc;
	std::string animation;
	int hp;
	if (!lua_isuserdata(L, 1))
	{
		luaL_typeerror(L, 1, "level pointer");
		return -1;
	}
	else
	{
		level = static_cast<Level*> (lua_touserdata(L, 1));
		if (!level)
		{
			luaL_typeerror(L, 1, "level pointer");
			return -1;
		}
	}
	if (!lua_isstring(L, 2))
	{
		luaL_typeerror(L, 2, "string");
		return -1;
	}
	else
	{
		sprite_id = lua_tostring(L, 2);
	}
	if (!lua_tovector(L, 3, &loc))
	{
		luaL_typeerror(L, 3, "vector");
	}
	if (!lua_isstring(L, 4))
	{
		luaL_typeerror(L, 4, "string");
		return -1;
	}
	else
	{
		animation = lua_tostring(L, 4);
	}if (!lua_isstring(L, 5))
	{
		luaL_typeerror(L, 5, "number");
		return -1;
	}
	else
	{
		hp = lua_tonumber(L, 5);
	}
	
	Entity* e = level->SpawnEntity(&(level->game->game_assets.spritesets[sprite_id]), loc, animation);
	e->hp = hp;
	lua_pushlightuserdata(L, e);
	return 1;
}

int LuaEngine::wrap_PolyAddVertex(lua_State* L)
{
	if (lua_gettop(L) != 2)
	{
		luaL_argerror(L, 2, "polygon pointer, vector");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Polygon* polygon = static_cast<Polygon*> (lua_touserdata(L, 1));
		if (polygon)
		{
			Vector2 v;
			if (lua_tovector(L, 2, &v))
			{
				polygon->vertices.push_back(v);
				return 0;
			}
		}
		luaL_typeerror(L, 1, "Unknown object type");
		return -1;
	}
	luaL_typeerror(L, 1, "Reference to level object expected");
	return -1;
}

int LuaEngine::wrap_LevelTriangulate(lua_State* L)
{
	if (lua_gettop(L) != 1)
	{
		luaL_argerror(L, 1, "level pointer");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Level* level = static_cast<Level*> (lua_touserdata(L, 1));
		if (level)
		{
			level->Triangulate();
			return 0;
		}
		luaL_typeerror(L, 1, "Expected level pointer");
		return -1;
	}
	luaL_typeerror(L, 1, "Reference to level object expected");
	return -1;
}

int LuaEngine::wrap_LevelCalculatePathLengths(lua_State* L)
{
	if (lua_gettop(L) != 1)
	{
		luaL_argerror(L, 1, "level pointer");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Level* level = static_cast<Level*> (lua_touserdata(L, 1));
		if (level)
		{
			level->CalculatePathLengths();
			return 0;
		}
		luaL_typeerror(L, 1, "Expected level pointer");
		return -1;
	}
	luaL_typeerror(L, 1, "Reference to level object expected");
	return -1;
}


int LuaEngine::wrap_CreatePathFollower(lua_State* L)
{
	if (lua_gettop(L) != 4)
	{
		luaL_argerror(L, 1, "level pointer, dynamic pointer, path pointer, number velocity");
		return -1;
	}
	Level* level;
	Dynamic* dynamic_obj;
	Path* path;
	float svelocity;
	mcPathFollower* controller;
	if (!lua_isuserdata(L, 1))
	{
		luaL_typeerror(L, 1, "Expected pointer");
		return -1;
	}
	else
	{
		level = static_cast<Level*> (lua_touserdata(L, 1));
		if (!level)
		{
			luaL_typeerror(L, 1, "Invalid level pointer");
			return -1;
		}
	}


	if (!lua_isuserdata(L, 2))
	{
		luaL_typeerror(L, 2, "Expected pointer");
		return -1;
	}
	else
	{
		dynamic_obj = static_cast<Dynamic*> (lua_touserdata(L, 2));
		if (!dynamic_obj)
		{
			luaL_typeerror(L, 2, "dynamic pointer");
			return -1;
		}
	}

	if (!lua_isuserdata(L, 3))
	{
		luaL_typeerror(L, 3, "Expected pointer");
		return -1;
	}
	else
	{
		path = static_cast<Path*> (lua_touserdata(L, 3));
		if (!path)
		{
			luaL_typeerror(L, 2, "Invalid path pointer");
			return -1;
		}
	}
	if (!lua_isnumber(L, 4))
	{
		luaL_typeerror(L, 4, "Expected number");
		return -1;
	}
	else
	{
		svelocity = lua_tonumber(L, 4);
	}

	
	controller = new mcPathFollower(dynamic_obj, path, svelocity);
	level->future_movement_controllers.push_back(controller);
	lua_pushlightuserdata(L, controller);
	return 1;
}

int LuaEngine::wrap_Travel(lua_State* L)
{
	if (lua_gettop(L) != 2)
	{
		luaL_argerror(L, 1, "movement controller pointer, number distance");
		return -1;
	}
	MovementController* mc;
	float to_distance;

	if (!lua_isuserdata(L, 1))
	{
		luaL_typeerror(L, 1, "Expected pointer");
		return -1;
	}
	else
	{
		mc = static_cast<MovementController*> (lua_touserdata(L, 1));
		if (!mc)
		{
			luaL_typeerror(L, 1, "Movement controller");
			return -1;
		}
	}

	if (!lua_isnumber(L, 2))
	{
		luaL_typeerror(L,2, "Expected number");
		return -1;
	}
	else
	{
		to_distance = lua_tonumber(L, 2);
	}
	mc->SetTravelTo(to_distance);
	return 0;
}



int LuaEngine::wrap_GetDistanceTraveled(lua_State* L)
{
	if (lua_gettop(L) != 1)
	{
		luaL_argerror(L, 1, "movement controller pointer");
		return -1;
	}
	mcPathFollower* mc;

	if (!lua_isuserdata(L, 1))
	{
		luaL_typeerror(L, 1, "Expected pointer");
		return -1;
	}
	else
	{
		mc = static_cast<mcPathFollower*> (lua_touserdata(L, 1));
		if (!mc)
		{
			luaL_typeerror(L, 1, "Movement controller");
			return -1;
		}
	}
	lua_pushnumber(L, mc->distance_traveled);
	return 1;
}

int LuaEngine::wrap_CreateEventHandler(lua_State* L)
{
	if (lua_gettop(L) != 1)
	{
		luaL_argerror(L, 1, "level pointer");
		return -1;
	}
	if (lua_isuserdata(L, 1))
	{
		Level* level = static_cast<Level*> (lua_touserdata(L, 1));
		if (level)
		{
			LuaEventHandler* event = new LuaEventHandler(&(level->game->lua));
			level->lua_event_handlers.push_back(event);
			lua_pushlightuserdata(L, event);
			return 1;
		}
		luaL_typeerror(L, 1, "Unknown object type");
		return -1;
	}
	luaL_typeerror(L, 1, "Reference to level object expected");
	return -1;
}


/*
	
	Register Event Wrappers

*/

int LuaEngine::wrap_OnAfter(lua_State* L)
{
	if (lua_gettop(L) != 3)
	{
		luaL_argerror(L, 1, "seconds, func pointer");
		return -1;
	}
	LuaEventHandler* handler;
	LuaEventTimer* timer;
	float time;
	int callback;
	if (!lua_isuserdata(L, 1))
	{
		luaL_typeerror(L, 1, "Expected pointer");
		return -1;
	}
	else
	{
		handler = static_cast<LuaEventHandler*> (lua_touserdata(L, 1));
		if (!handler)
		{
			luaL_typeerror(L, 1, "event handler");
			return -1;
		}
	}
	if (!lua_isnumber(L, 2))
	{
		luaL_typeerror(L, 2, "Expected number");
		return -1;
	}
	else
	{
		time = lua_tonumber(L, 2);
	}
	if (!lua_isfunction(L, 3))
	{
		luaL_typeerror(L, 3, "Expected function");
		return -1;
	}
	else
	{
		callback = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	timer = new LuaEventTimer(time);
	timer->lua_callback = callback;
	handler->RegisterEvent(timer);
	return 0;
}

int LuaEngine::wrap_OnTravelComplete(lua_State* L)
{
	if (lua_gettop(L) != 3)
	{
		luaL_argerror(L, 1, "event handler, movement controller, function ptr");
		return -1;
	}
	LuaEventHandler* handler;
	MovementController* mc;
	LuaEventTravelComplete* lua_event;
	int callback;
	if (!lua_isuserdata(L, 1))
	{
		luaL_typeerror(L, 1, "Expected pointer");
		return -1;
	}
	else
	{
		handler = static_cast<LuaEventHandler*> (lua_touserdata(L, 1));
		if (!handler)
		{
			luaL_typeerror(L, 1, "event handler");
			return -1;
		}
	}

	if (!lua_isuserdata(L, 2))
	{
		luaL_typeerror(L, 2, "movement controller pointer");
		return -1;
	}
	else
	{
		mc = static_cast<MovementController*> (lua_touserdata(L, 2));
		if (!mc)
		{
			luaL_typeerror(L, 2, "movement controller pointer");
			return -1;
		}
	}

	if (!lua_isfunction(L, 3))
	{
		luaL_typeerror(L, 3, "Expected function");
		return -1;
	}
	else
	{
		callback = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	lua_event = new LuaEventTravelComplete(mc);
	lua_event->lua_callback = callback;
	handler->RegisterEvent(lua_event);
	return 0;
}