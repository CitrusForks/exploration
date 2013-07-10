#include "stdafx.h"
#include "ScriptedScene.h"
#include "LunaShare.hpp"
#include "ScriptedActor.h"

using namespace std;
using namespace Luna;
using namespace DirectX;

const char ScriptedScene::className[] = "Scene"; // not "ScriptedScene" in the interest of brevity; bad idea? not sure..

// properties accessible from Lua:
const LunaShare<ScriptedScene>::PropertyType ScriptedScene::properties[] = 
{
    {0,0}
};


// methods accessible from Lua:
const LunaShare<ScriptedScene>::FunctionType ScriptedScene::methods[] = 
{
    {"exits", &ScriptedScene::l_exits},
    {"enters", &ScriptedScene::l_enters},
    {"configureLight", &ScriptedScene::l_configureLight},
    {"moveLight", &ScriptedScene::l_moveLight},
    {"pointMoonlight", &ScriptedScene::l_pointMoonlight},
    {"getModel", &ScriptedScene::l_getModel},
    {"updateFlashlight", &ScriptedScene::l_updateFlashlight},
    {0,0}
};


ScriptedScene::ScriptedScene(lua_State *LL) : L(LL)
{
    D3DClass *d3d = nullptr;

    lua_getglobal(L, "d3d");
    d3d = static_cast<D3DClass *>(lua_touserdata(L, -1));
    lua_pop(L, 1); // pop d3d

    if (d3d == nullptr) 
    {
        Errors::Cry("Must set d3d in Lua global namespace; the engine should be doing this automatically :(");
    }

    shared_ptr<ScriptedCamera> *camera = (shared_ptr<ScriptedCamera> *) luaL_checkudata(L, 1, ScriptedCamera::className);

    FPCam = *camera;

    cout << "ScriptedScene initializing!" << endl;

    init(d3d);
}


ScriptedScene::~ScriptedScene(void)
{
}


// Scene:enters(Actor a)
// returns int reference to actor; useful for retrieving the object from Scene later and calling exits()
int ScriptedScene::l_enters( lua_State *L )
{
    int sp = lua_gettop(L);
    
    // add actor
    shared_ptr<Actor> *u = static_cast<shared_ptr<Actor> *>(luaL_checkudata(L, 2, ScriptedActor::className)); // using luaL_checkudata() over luaL_testudata() here for simplicity; it shouldn't be that hard to pass a valid object to this thing
    int refNum = enters(*u);

    assert(lua_gettop(L) == sp); // assert correct stack depth

    lua_pushinteger(L, refNum);

    return 1; // number of results
}

// remove an object from the world
int ScriptedScene::l_exits( lua_State *L )
{
    unsigned i = (unsigned int) luaL_checkinteger(L, 1);

    exits(i);

    return 0; // number of results
}

// arguments: index, pos_x, pos_y, pos_z, [optional: dir_x, dir_y, dir_z]
int ScriptedScene::l_moveLight( lua_State *L )
{
    int idx = luaL_checkinteger(L, 2);

    XMVECTOR pos = XMVectorSet((float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), (float)luaL_checknumber(L, 5), 1);
    XMVECTOR dir = (lua_gettop(L) == 8) ? XMVectorSet((float)luaL_checknumber(L, 4), (float)luaL_checknumber(L, 5), (float)luaL_checknumber(L, 6), 1) :
        XMLoadFloat3(&(m_lighting->getLights()[idx].direction)); // XXX this shouldn't be necessary perhaps?

    m_lighting->getLights()[idx].move(pos, dir);

    return 0; // number of results
}


int ScriptedScene::l_configureLight( lua_State *L )
{
//void LightsAndShadows::setSpotlight( int which, DirectX::FXMVECTOR position, DirectX::FXMVECTOR direction, float beamHalfAngle /*= (float)M_PI * 15.0f/180.f*/, float constantAttenuation /*= 0.75f*/, float linearAttenuation /*= 0.0f*/, float quadraticAttenuation /*= 0.025f*/ )

    int idx = luaL_checkinteger(L, 2);

    m_lighting->setSpotlight(idx, 
        XMVectorSet((float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), (float)luaL_checknumber(L, 5), 1),
        XMVectorSet((float)luaL_checknumber(L, 4), (float)luaL_checknumber(L, 5), (float)luaL_checknumber(L, 6), 1),
        (float)luaL_checknumber(L, 7),
        (float)luaL_checknumber(L, 8),
        (float)luaL_checknumber(L, 9),
        (float)luaL_checknumber(L, 10)
        );

    return 0; // as always, number of results
}

// takes the name of a 3D object file as an argument
// returns model index
int ScriptedScene::l_getModel( lua_State *L )
{
    const char * const modelName = luaL_checkstring(L, -1);

    int refNum = m_models->getRefNum(modelName);

    if (!refNum)
    {
        cerr << "Could not load " << modelName << endl;
    }

    lua_pushinteger(L, refNum);

    return 1;
}

// runs the update(now, sinceLastFrame) function in the Lua global namespace
bool ScriptedScene::update( float now, float timeSinceLastUpdate, std::shared_ptr<FirstPerson> dummy /*= nullptr*/ )
{
    lua_getglobal(L, "update");
    if (!lua_isfunction(L, -1)) Errors::Cry("Lua script needs an update() function.");

    lua_pushnumber(L, now);
    lua_pushnumber(L, timeSinceLastUpdate);

    if (lua_pcall(L, 2, 0, 0) != 0)
    {
        Errors::Cry((char*)lua_tostring(L, -1));
    }

    return 0;
}
// 3 arguments, x y z, establishing a vector parallel to the directional light in the scene
int ScriptedScene::l_pointMoonlight( lua_State *L )
{
    XMVECTOR dir = XMVector3Normalize(XMVectorSet((float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), 0));

    m_lighting->pointMoonlight(dir, *FPCam);

    return 0;
}

// takes the index of the spotlight to move to current correct flashlight position
// and an optional beam halfangle
int ScriptedScene::l_updateFlashlight( lua_State *L )
{
    int idx = luaL_checkinteger(L, 2);
    float beamHalfAngle = (float)(M_PI * 15.0 / 180);


    if (lua_gettop(L) == 3)
    {
        beamHalfAngle = (float)luaL_checknumber(L, 3);
    }

    m_lighting->updateFlashlight(*FPCam, beamHalfAngle, idx);

    return 0;
}
