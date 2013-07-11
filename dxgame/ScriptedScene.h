#pragma once

#include "scene.h"
#include <lua.hpp>
#include "LunaShare.hpp"
#include <memory>

class ScriptedScene :
    public Scene
{
public:

    static const char className[];
    static const Luna::LunaShare<ScriptedScene>::PropertyType properties[];
    static const Luna::LunaShare<ScriptedScene>::FunctionType methods[];

    std::shared_ptr<ScriptedCamera> FPCam;

    ScriptedScene(lua_State *LL);
    ~ScriptedScene(void);

    virtual bool update(float now, float timeSinceLastUpdate, std::shared_ptr<FirstPerson> dummy = nullptr); // same header as Scene; still thinking about camera object handling 


    // Lua-centric methods:
    int l_enters(lua_State *L); // adds an actor 
    int l_exits(lua_State *L);  // removes an actor, pursued by bear
    int l_exeunt(lua_State *L); // removes all actors 
    int l_moveLight(lua_State *L); // moves a light; NOTE perhaps make lights their own objects in Lua as well? Though this is easier, of course.
    int l_configureLight(lua_State *L);  // set RGB value, orientation, and beam angle for light
    int l_getModel(lua_State *L); // look up a model by name in the ModelManager; this is a shortcut to avoid turning ModelManager into its own Lua object
    int l_pointMoonlight(lua_State *L); // sets the direction of the directional light source. TODO: need to set its color and have the option to disable it for indoor scenes
    int l_updateFlashlight(lua_State *L); // sets the position of a spotlight to the correct position to act as the player's flashlight; must run every update 

private:
    lua_State *L; // caching this value but perhaps it's terrible style? TODO.
};

