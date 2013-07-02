#pragma once
#include <memory>
#include <lunafive.hpp>

class LuaSharedPointerActorWrapper : public std::shared_ptr<Actor>
{
public:

    static const char className[];
    static const Luna<LuaSharedPointerActorWrapper>::PropertyType properties[];
    static const Luna<LuaSharedPointerActorWrapper>::FunctionType methods[];

    LuaSharedPointerActorWrapper(lua_State *LL);
};

