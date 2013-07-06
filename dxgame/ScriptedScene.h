#pragma once

#include "scene.h"
#include <lua.hpp>
#include "LunaShare.hpp"

class ScriptedScene :
    public Scene
{
public:

    static const char className[];
    static const Luna::LunaShare<ScriptedScene>::PropertyType properties[];
    static const Luna::LunaShare<ScriptedScene>::FunctionType methods[];

    ScriptedScene(lua_State *LL);
    ~ScriptedScene(void);

    // Lua-centric methods:
    int l_enters(lua_State *L); // adds an actor TODO: come up with precise parameter lists
    int l_exits(lua_State *L);  // removes an actor
    int l_moveLight(lua_State *L);
    int l_configureLight(lua_State *L);
    int l_getModel(lua_State *L);

private:
    lua_State *L;
};

