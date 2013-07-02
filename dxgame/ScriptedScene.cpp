#include "stdafx.h"
#include "ScriptedScene.h"
#include "LunaShare.hpp"
#include "ScriptedActor.h"

using namespace std;
using namespace Luna;

const char ScriptedScene::className[] = "Scene"; // not "ScriptedScene" in the interest of brevity; bad idea? not sure..

// properties accessible from Lua:
const LunaShare<ScriptedScene>::PropertyType ScriptedScene::properties[] = 
{
    {0,0}
};


// methods accessible from Lua:
const LunaShare<ScriptedScene>::FunctionType ScriptedScene::methods[] = 
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


// Scene:enters(Actor a)
// returns int reference to actor; useful for retrieving the object from Scene later and calling exits()
int ScriptedScene::l_enters( lua_State *L )
{
    int sp = lua_gettop(L);
    
    // add actor
    shared_ptr<Actor> *u = static_cast<shared_ptr<Actor> *>(luaL_checkudata(L, 1, ScriptedActor::className)); // using luaL_checkudata() over luaL_testudata() here for simplicity; it shouldn't be that hard to pass a valid object to this thing
    int refNum = enters(*u);

    assert(lua_gettop(L) == sp); // assert correct stack depth

    lua_pushinteger(L, refNum);

    return 1; // number of results
}


int ScriptedScene::l_exits( lua_State *L )
{
    unsigned i = (unsigned int) luaL_checkinteger(L, 1);

    exits(i);

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
