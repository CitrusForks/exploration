#pragma once

#include <vector>
#include <memory>
#include "Actor.h"
#include "LightsAndShadows.h"
#include "ModelManager.h"
#include "TextureManager.h"

// a scene just contains actors
// ...perhaps an additional Lua script?
class Scene
{
public:

    typedef std::function<bool(DirectX::CXMMATRIX world, shared_ptr<ModelManager> models, int modelRefNum, shared_ptr<LightsAndShadows> lighting)> 
        renderFunc_t;

    // draw all actors:
    bool Scene::render( renderFunc_t &renderFunc );

    virtual bool update(float now, float timeSinceLastUpdate, FirstPerson &FPCamera); 

    int enters(shared_ptr<Actor> actor); // adds an actor, named with theater term for no good reason
    void exits(unsigned int actor); // I hope you saved that reference from the enters() call...

    // we need our abstraction of the graphics API too much and everywhere, hence we keep a pointer to D3DClass for convenience
    // passing lighting is optional; a copy will be created if you pass in null
    // same with models and textures
    Scene(D3DClass &d3d, shared_ptr<LightsAndShadows> lighting = nullptr, shared_ptr<ModelManager> models = nullptr, shared_ptr<TextureManager> textures = nullptr);
    virtual ~Scene(void);

    shared_ptr<LightsAndShadows> getLights() { return m_lighting; } // flexible design (tm)

protected:
    std::vector <shared_ptr<Actor>> m_actors;
    shared_ptr<LightsAndShadows> m_lighting;
    shared_ptr<ModelManager> m_models;
    shared_ptr<TextureManager> m_textures;
    D3DClass &m_d3d;
};

