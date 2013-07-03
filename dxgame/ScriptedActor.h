#pragma once
#include <memory>
#include <lua.h>
#include "LunaShare.hpp"

#include "Actor.h"

class ScriptedActor : public Actor
{
public:

    static const char className[];
    static const Luna::LunaShare<ScriptedActor>::PropertyType properties[];
    static const Luna::LunaShare<ScriptedActor>::FunctionType methods[];

    // Lua-facing methods
    int l_moveTo(lua_State *L); // arg: x, y, z
    int l_setRollPitchYaw(lua_State *L); // arg: roll, pitch, yaw
    int l_slerp(lua_State *L); // arg: roll, pitch, yaw

    ScriptedActor(lua_State *L);
};

