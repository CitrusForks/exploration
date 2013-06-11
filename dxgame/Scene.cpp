#include "stdafx.h"

#include <memory>

#include "Scene.h"
#include "d3dclass.h"
#include "LightsAndShadows.h"
#include "ModelManager.h"
#include "TextureManager.h"


Scene::Scene(D3DClass &d3d, shared_ptr<LightsAndShadows> lighting /* = nullptr */, shared_ptr<ModelManager> models /* = nullptr */, shared_ptr<TextureManager> textures /* = nullptr */)
    : m_d3d(d3d)
{

    m_models = models ? models : make_shared<ModelManager>(d3d);
    m_textures = textures ? textures : make_shared<TextureManager>();
    m_lighting = lighting ? lighting : shared_ptr<LightsAndShadows>(new LightsAndShadows(d3d));
}


Scene::~Scene(void)
{
}

bool Scene::update( float now, float timeSinceLastUpdate, FirstPerson &FPCamera )
{
    for (auto &i: m_actors)
    {
        bool rc = true;
        if (i) rc = i->update(now, timeSinceLastUpdate);
        if (!true)
        {
            i = nullptr;
        }
    }

    return true;
}

// render all actors
// renderFunc is a function that encapsulates the entire graphics engine state needed to render a frame except for the data in Scene
bool Scene::render( renderFunc_t &renderFunc )
{
    auto lighting = m_lighting; // copy in local scope to allow capture by lambda expression
    auto models = m_models;
    Actor::renderFunc_t renderFuncForActor = [=, &renderFunc] (DirectX::CXMMATRIX world, int modelRefNum)
    {
        return renderFunc(world, models, modelRefNum, lighting);
    };

    for (auto &i: m_actors)
    {
        if (i) if(!i->render(renderFuncForActor)) return false;
    }

    return true;
}

int Scene::enters( shared_ptr<Actor> actor )
{
    m_actors.push_back(actor);
    actor->setID(m_actors.size()-1);
    return m_actors.size()-1;
}

void Scene::exits( unsigned int actor )
{
    assert(actor < (m_actors.size()));
    m_actors[actor] = nullptr;
}
