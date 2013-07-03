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
    {
        "moveTo", &ScriptedActor::l_moveTo
    },
    {0,0}
};

const LunaShare<ScriptedActor>::PropertyType ScriptedActor::properties[] = 
{
    {0,0}
};

ScriptedActor::ScriptedActor( lua_State *L )
{
    init(0);
}

int ScriptedActor::l_moveTo( lua_State *L )
{
    // cout << "wat: " << lua_gettop(L);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);
    // cout << " " << x << " " << y << " " << z << endl;

    moveTo(XMVectorSet(x, y, z, 1.0f));

    return 0;
}

