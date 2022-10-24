#pragma once
#include <iostream>


extern "C"
{
#include "../lua/include/lua.h"
#include "../lua/include/lauxlib.h"
#include "../lua/include/lualib.h"

}


int lua_HostFunction(lua_State* L);
class Game;

class LuaEngine
{
private:
	lua_State* L;
	Game* game = nullptr;
public:
	LuaEngine()
	{
		L = luaL_newstate();
		luaL_openlibs(L);
		lua_register(L, "HostFunction", lua_HostFunction);
		lua_register(L, "CreateLevel", wrap_CreateLevel);
		lua_register(L, "CreatePath", wrap_CreatePath);
		lua_register(L, "CreateStaticCollisionPolygon", wrap_CreateStaticCollisionPolygon);
		lua_register(L, "CreatePathFollower", wrap_CreatePathFollower);
		lua_register(L, "SpawnEnemy", wrap_SpawnEnemy);

		lua_register(L, "PathBuildLine", wrap_PathBuildLine);
		lua_register(L, "PathBuildCurve", wrap_PathBuildCurve);
		lua_register(L, "PathBuildControlPoint", wrap_PathBuildControlPoint);
		lua_register(L, "PathGetLength", wrap_PathGetLength);
		lua_register(L, "Travel", wrap_Travel);
		lua_register(L, "GetDistanceTraveled", wrap_GetDistanceTraveled);

		lua_register(L, "LevelAddBackground", wrap_LevelAddBackground);
		lua_register(L, "LevelSetMainPath", wrap_LevelSetMainPath);
		lua_register(L, "LevelTriangulate", wrap_LevelTriangulate);
		lua_register(L, "LevelCalculatePathLengths", wrap_LevelCalculatePathLengths);
		
		lua_register(L, "PolyAddVertex", wrap_PolyAddVertex);
		
		lua_register(L, "CreateEventHandler", wrap_CreateEventHandler);
		lua_register(L, "OnAfter", wrap_OnAfter);
		lua_register(L, "OnTravelComplete", wrap_OnTravelComplete);

		lua_register(L, "EnemyGetCollisionPolygon", wrap_EnemyGetCollisionPolygon);
		lua_register(L, "EnemySetLocationOffset", wrap_EnemySetLocationOffset);
	}
	void SetGame(Game* g);
	static bool lua_tovector(lua_State* L, int index, Vector2* v);
	static int wrap_CreateLevel(lua_State* L);
	static int wrap_CreatePath(lua_State* L);
	static int wrap_CreateStaticCollisionPolygon(lua_State* L);
	static int wrap_CreatePathFollower(lua_State* L);
	static int wrap_SpawnEnemy(lua_State* L);

	static int wrap_LevelAddBackground(lua_State* L);
	static int wrap_LevelSetMainPath(lua_State* L);
	static int wrap_LevelTriangulate(lua_State* L);
	static int wrap_LevelCalculatePathLengths(lua_State* L);
	

	static int wrap_PolyAddVertex(lua_State* L);

	static int wrap_PathBuildLine(lua_State* L);
	static int wrap_PathBuildCurve(lua_State* L);
	static int wrap_PathBuildControlPoint(lua_State* L);
	static int wrap_PathGetLength(lua_State* L);

	static int wrap_Travel(lua_State* L);
	static int wrap_GetDistanceTraveled(lua_State* L);
	
	static int wrap_CreateEventHandler(lua_State* L);
	static int wrap_OnAfter(lua_State* L);
	static int wrap_OnTravelComplete(lua_State* L);

	static int wrap_EnemyGetCollisionPolygon(lua_State* L);
	static int wrap_EnemySetLocationOffset(lua_State* L);

	void LoadLevel(const char* lua_file);
	void CallFunction(std::string function_name);
	
	bool Callback(int r)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, r);
		bool success = CheckLua(lua_pcall(L, 0, 0, 0));
		
		return success;
	}
	void DerefPtr(int r)
	{
		luaL_unref(L, LUA_REGISTRYINDEX, r);
	}
	static void log_error(std::string msg)
	{
		std::cout << "[ERROR] " << msg << std::endl;
	}
	bool CheckLua(int r)
	{
		if (r != LUA_OK)
		{
			std::string errormsg = lua_tostring(L, -1);
			log_error(errormsg);
			return false;
		}
		return true;
	}
	void resume_coroutine(void* ptr)
	{
		
		lua_getglobal(L, "resume_coroutine");
		if (lua_isfunction(L, -1))
		{
			lua_pushlightuserdata(L, ptr);
			
			if (CheckLua(lua_pcall(L, 1, 1, 0))) // takes 1 argument, returns 1
			{
				
			}
			
		}
	}
	~LuaEngine()
	{
		lua_close(L);
	}
};


class LuaEvent
{
public:
	virtual bool evaluate() = 0;
	int lua_callback;
	bool one_shot = false;
	virtual bool is_one_shot() const { return one_shot; }
};


class LuaEventTravelComplete: public LuaEvent
{
	bool one_shot = true;
	MovementController* movement_controller;
public:
	virtual bool is_one_shot() const { return one_shot; }
	LuaEventTravelComplete(MovementController* mc) {
		movement_controller = mc;
	}
	bool evaluate() override {
		return movement_controller->IsTravelComplete();
	}
};

class LuaEventTimer : public LuaEvent
{
	bool one_shot = true;
	virtual bool is_one_shot() const { return one_shot; }
	Uint64 end_time = 0;
public:
	LuaEventTimer (float delay)
	{
		end_time = SDL_GetPerformanceCounter() + (delay * SDL_GetPerformanceFrequency());
	}
	LuaEventTimer()
	{
		end_time = SDL_GetPerformanceCounter();
	}
	bool evaluate() override {
		if (SDL_GetPerformanceCounter() > end_time) return true;
		return false;
	}
};

class LuaEventHandler
{
	std::vector<LuaEvent*> events;
	std::vector<LuaEvent*> future_events;
	LuaEngine* lua;
public:
	LuaEventHandler(LuaEngine* l)
	{
		lua = l;
	}
	void RegisterEvent(LuaEvent* e)
	{
		future_events.push_back(e);
	}

	void UpdateEvents()
	{
		for (auto& e : future_events)
		{
			events.push_back(e);
		}
		future_events.clear();

		
		std::vector<LuaEvent*>::iterator it = events.begin();
		while (it != events.end())
		{

			if ((*it)->evaluate())
			{
				lua->Callback((*it)->lua_callback);
				if ((*it)->is_one_shot())
				{
					lua->DerefPtr((*it)->lua_callback);
					it = events.erase(it);
				}
				else it++;
			}
			else it++;
		}
	}

};