#include "stdafx.h"
#include "ScriptedScene.h"
#include <lunafive.hpp>


using namespace std;

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


ScriptedScene::ScriptedScene(lua_State *LL) : L(LL)
{
    D3DClass *d3d = nullptr;

    lua_getglobal(L, "d3d");
    d3d = static_cast<D3DClass *>(lua_touserdata(L, -1));

    if (d3d == nullptr) 
    {
        Errors::Cry("Must set d3d on Lua stack :(");
    }

    cout << "ScriptedScene initializing!" << endl;

    init(d3d);
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
