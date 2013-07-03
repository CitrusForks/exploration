#include "stdafx.h"
#include "ScriptedActor.h"
#include <memory>
#include "Actor.h"

using namespace std;
using namespace DirectX;
using namespace Luna;

const char ScriptedActor::className[] = "Actor";

const LunaShare<ScriptedActor>::FunctionType ScriptedActor::methods[] = 
{
    {0,0}
};

const LunaShare<ScriptedActor>::PropertyType ScriptedActor::properties[] = 
{
    {0,0}
};

ScriptedActor::ScriptedActor( lua_State *L )
{

}

