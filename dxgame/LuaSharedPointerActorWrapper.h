#pragma once
#include <memory>
#include <lunafive.hpp>
#include <lua.h>

class LuaSharedPointerActorWrapper : public std::shared_ptr<Actor>
{
public:

    static const char className[];
    static const Luna<LuaSharedPointerActorWrapper>::PropertyType properties[];
    static const Luna<LuaSharedPointerActorWrapper>::FunctionType methods[];

    // Lua-facing methods
    int l_moveTo(lua_State *L); // arg: x, y, z
    int l_setRollPitchYaw(lua_State *L); // arg: roll, pitch, yaw
    int l_slerp(lua_State *L); // arg: roll, pitch, yaw

    LuaSharedPointerActorWrapper(lua_State *LL);
};

