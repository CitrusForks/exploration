#include "stdafx.h"

#include <memory>

#include "Scene.h"
#include "d3dclass.h"
#include "LightsAndShadows.h"
#include "ModelManager.h"
#include "TextureManager.h"


void Scene::init(D3DClass *d3d, shared_ptr<LightsAndShadows> lighting /* = nullptr */, shared_ptr<ModelManager> models /* = nullptr */, shared_ptr<TextureManager> textures /* = nullptr */)
{

    m_models = models ? models : make_shared<ModelManager>(*d3d);
    m_textures = textures ? textures : make_shared<TextureManager>();
    m_lighting = lighting ? lighting : shared_ptr<LightsAndShadows>(new LightsAndShadows(*d3d));
    m_d3d = d3d;

    m_skyBoxTexture = nullptr;
    setSkyBox(L"GrimmNight_cube.dds"); // deeefffaaaaaaaaaaauult
}


Scene::Scene()
{

}


Scene::Scene( D3DClass *d3d, shared_ptr<LightsAndShadows> lighting /*= nullptr*/, shared_ptr<ModelManager> models /*= nullptr*/, shared_ptr<TextureManager> textures /*= nullptr*/ )
{
    init(d3d, lighting, models, textures);
}


Scene::~Scene(void)
{
    m_actors.clear();
    m_lighting = nullptr;
    m_models = nullptr;
    m_textures = nullptr;

    cout << "~Scene() called, fyi." << endl;
}


bool Scene::update( float now, float timeSinceLastUpdate, std::shared_ptr<FirstPerson> unused )
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
// renderFunc is a function that encapsulates the entire graphics engine state needed to render a frame except for the data in Scene and Actor[s]
bool Scene::render( renderFunc_t &renderFunc )
{
    using namespace std::placeholders;
    Actor::renderFunc_t partialBind = std::bind(renderFunc, _1, m_models, _2, m_lighting);

    for (auto &i: m_actors)
    {
        if (i) if(!i->render(partialBind)) return false;
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

void Scene::replaceManagers( std::shared_ptr<ModelManager> mm, std::shared_ptr<TextureManager> tm )
{
    m_textures = tm;
    m_models = mm;
}

// this is most likely to be called internally within ScriptedScene or something but it's implemented here anyway because it seems like the place for it
void Scene::setSkyBox( std::wstring textureFileName )
{
    m_skyBoxTexture = make_shared<LoadedTexture>(); // is shared_ptr making me lazy? eh.

    try
    {
        m_textures->getTexture(textureFileName, m_d3d->GetDevice(), m_d3d->GetDeviceContext(), *m_skyBoxTexture);
    }
    catch (exception*)
    {
    	m_skyBoxTexture = nullptr; // meh
    }
}
