#include "stdafx.h"
#include "LuaSharedPointerActorWrapper.h"
#include <memory>
#include "Actor.h"

const char LuaSharedPointerActorWrapper::className[] = "Actor";

const Luna<LuaSharedPointerActorWrapper>::FunctionType LuaSharedPointerActorWrapper::methods[] = 
{
    {0,0}
};

const Luna<LuaSharedPointerActorWrapper>::PropertyType LuaSharedPointerActorWrapper::properties[] = 
{
    {0,0}
};


LuaSharedPointerActorWrapper::LuaSharedPointerActorWrapper(shared_ptr<Actor> actor) : m_actor(actor)
{
}


LuaSharedPointerActorWrapper::~LuaSharedPointerActorWrapper(void)
{
}

