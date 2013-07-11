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
    {
        "setRollPitchYaw", &ScriptedActor::l_setRollPitchYaw
    },
    {0,0}
};

const LunaShare<ScriptedActor>::PropertyType ScriptedActor::properties[] = 
{
    {0,0}
};

ScriptedActor::ScriptedActor( lua_State *L )
{
    int sp = lua_gettop(L);
    int modelRefNum = 0;

    modelRefNum = luaL_checkinteger(L, 1);

    XMMATRIX correction = XMMatrixIdentity();

    if (lua_gettop(L) >= 5)
    {
        // correctional rotation: axis, angle
        XMVECTOR axis = XMVectorSet((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), 0);
        correction *= XMMatrixRotationAxis(axis,
            (float)luaL_checknumber(L, 5)); // angle

        if (lua_gettop(L) >= 6)
        {
            // correctional rescale, by a single factor to keep this uniform
            float scale = (float)luaL_checknumber(L, 6);
            if (!scale)
            {
                cerr << "Scale factor of 0.0f is probably an error." << endl;
            }

            correction *= XMMatrixScaling(scale, scale, scale);
        }

        if (lua_gettop(L) >= 9)
        {
            // correctional offset

            correction *= XMMatrixTranslation((float)luaL_checknumber(L, 7), (float)luaL_checknumber(L, 8), (float)luaL_checknumber(L, 9));
        }
    }

    init(modelRefNum, correction);
    assert(sp == lua_gettop(L));
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

// roll, pitch, yaw, like it says in the name.
int ScriptedActor::l_setRollPitchYaw( lua_State *L )
{
    float roll = (float)luaL_checknumber(L, 2);
    float pitch   = (float)luaL_checknumber(L, 3);
    float yaw  = (float)luaL_checknumber(L, 4);

    setPitchYawRoll(pitch, yaw, roll);

    return 0;
}

