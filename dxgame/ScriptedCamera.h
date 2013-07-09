#pragma once

#include "firstperson.h"
#include <lua.h>
#include "LunaShare.hpp"

// exposing the camera to Lua

class ScriptedCamera :
    public FirstPerson
{
public:

    static const char className[];
    static const Luna::LunaShare<ScriptedCamera>::PropertyType properties[];
    static const Luna::LunaShare<ScriptedCamera>::FunctionType methods[];

    int l_setPosition(lua_State *L);
    int l_setYawPitch(lua_State *L);

    int l_getPosition(lua_State *L);
    int l_getYawPitch(lua_State *L);

    ScriptedCamera(lua_State *L = nullptr); // lua_State should be optional here
    ~ScriptedCamera(void);
};

