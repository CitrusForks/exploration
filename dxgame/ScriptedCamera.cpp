#include "stdafx.h"
#include "ScriptedCamera.h"
#include <DirectXMath.h>

using namespace DirectX;

// constants for Luna:
const char ScriptedCamera::className[] = "Camera";

const Luna::LunaShare<ScriptedCamera>::FunctionType ScriptedCamera::methods[] = 
{
    {
        "setPosition", &ScriptedCamera::l_setPosition
    },
    {
        "setYawPitch", &ScriptedCamera::l_setYawPitch
    },
    {
        "getPosition",  &ScriptedCamera::l_getPosition
    },
    {
        "getYawPitch", &ScriptedCamera::l_getYawPitch
    },
    0, 0
};

const Luna::LunaShare<ScriptedCamera>::PropertyType ScriptedCamera::properties[] = 
{
    0, 0
};




ScriptedCamera::ScriptedCamera(lua_State *L) : FirstPerson()
{
}


ScriptedCamera::~ScriptedCamera(void)
{
}

int ScriptedCamera::l_setPosition( lua_State *L )
{
    float x = (float)luaL_checknumber(L, 2); // 1 is self
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);

    setPosition(XMVectorSet(x, y, z, 1));

    return 0;
}

int ScriptedCamera::l_setYawPitch( lua_State *L )
{
    m_heading = (float)luaL_checknumber(L, 2); // 1 is self
    m_pitch = (lua_gettop(L) > 2 && lua_isnumber(L, 3)) ? (float) lua_tonumber(L, 3) : m_pitch;

    return 0;
}

int ScriptedCamera::l_getPosition( lua_State *L )
{
    lua_pushnumber(L, m_position.x);
    lua_pushnumber(L, m_position.y);
    lua_pushnumber(L, m_position.z);

    return 3;
}

int ScriptedCamera::l_getYawPitch( lua_State *L )
{
    lua_pushnumber(L, m_heading);
    lua_pushnumber(L, m_pitch);

    return 2;
}

