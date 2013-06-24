#include "stdafx.h"
#include "ScriptedScene.h"
#include <lunafive.hpp>


const char ScriptedScene::className[] = "Scene"; // not "ScriptedScene" in the interest of brevity; bad idea? not sure..

// properties accessible from Lua:
const Luna<ScriptedScene>::PropertyType ScriptedScene::properties[] = 
{
    {0,0}
};

// methods accessible from Lua:
const Luna<ScriptedScene>::FunctionType ScriptedScene::methods[] = 
{
    {"exits", &ScriptedScene::l_exits},
    {"enters", &ScriptedScene::l_enters},
    {"configureLight", &ScriptedScene::l_configureLight},
    {"moveLight", &ScriptedScene::l_moveLight},
    {0,0}
};


ScriptedScene::ScriptedScene(lua_State *LL, D3DClass &d3d) : L(LL), Scene(d3d)
{
}


ScriptedScene::~ScriptedScene(void)
{
}

int ScriptedScene::l_enters( lua_State *L )
{
    int sp = lua_gettop(L);
    // add actor
    // assert correct stack depth
    return 0; // number of results
}

int ScriptedScene::l_exits( lua_State *L )
{
    return 0; // number of results
}

int ScriptedScene::l_moveLight( lua_State *L )
{
    return 0; // number of results
}

int ScriptedScene::l_configureLight( lua_State *L )
{
    return 0; // as always, number of results
}
