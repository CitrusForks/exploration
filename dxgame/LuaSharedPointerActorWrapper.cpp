#include "stdafx.h"
#include "LuaSharedPointerActorWrapper.h"
#include <memory>
#include "Actor.h"

using namespace std;
using namespace DirectX;

const char LuaSharedPointerActorWrapper::className[] = "Actor";

const Luna<LuaSharedPointerActorWrapper>::FunctionType LuaSharedPointerActorWrapper::methods[] = 
{
    {0,0}
};

const Luna<LuaSharedPointerActorWrapper>::PropertyType LuaSharedPointerActorWrapper::properties[] = 
{
    {0,0}
};

