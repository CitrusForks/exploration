#pragma once
#include <memory>
#include <lunafive.hpp>

class LuaSharedPointerActorWrapper
{
public:

    static const char className[];
    static const Luna<LuaSharedPointerActorWrapper>::PropertyType properties[];
    static const Luna<LuaSharedPointerActorWrapper>::FunctionType methods[];

    LuaSharedPointerActorWrapper(shared_ptr<Actor> actor);
    ~LuaSharedPointerActorWrapper(void);

private:
    shared_ptr<Actor> m_actor;
};

