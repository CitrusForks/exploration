#include "stdafx.h"
#include "SceneDemo.h"
#include <memory>
#include "TextureManager.h"
#include "ModelManager.h"
#include <DirectXMath.h>

using namespace DirectX;
using namespace std;

SceneDemo::SceneDemo(D3DClass &d3d, shared_ptr<ModelManager> models, shared_ptr<TextureManager> textures)
    : Scene(d3d, nullptr, models, textures)
{

    (*m_models)["Chekov.obj"];

    (*m_models)["duck.obj"]; // retrieving an unloaded model triggers a load from disk

    (*m_models)["floor.obj"];

    (*m_models)["torus.obj"];

    (*m_models)["LPBuildX13r_3ds.3ds"];

    (*m_models)["spooky_tree.obj"];
    // WARNING, pointers returned by models["whatever"] may be invalid later if a model is loaded and the internal vector is reallocated. Use refnums to store an index for longterm fast lookup!

    enters(chekov = shared_ptr<Actor>(
        new Actor(m_models->getRefNum("Chekov.obj"), XMMatrixScaling(0.53f, 0.53f, 0.53f))
        ));

    enters(duck = shared_ptr<Actor>(
        new Actor(m_models->getRefNum("duck.obj"), XMMatrixTranslation(0, -0.2f, 0))
        ));

    enters(shared_ptr<Actor>(
        new Actor(m_models->getRefNum("floor.obj"))
        ));

    enters(torus = shared_ptr<Actor>(
        new Actor(m_models->getRefNum("torus.obj"))
        ));

    enters(house = shared_ptr<Actor>(
        new Actor(m_models->getRefNum("LPBuildX13r_3ds.3ds"), XMMatrixRotationAxis(XMVectorSet(1.0f,0,0,0), (float)M_PI_2) * XMMatrixScaling(0.15f, 0.15f, 0.15f) * XMMatrixTranslation(0.0f, 4.5001f, 0))
        ));

    enters(tree = shared_ptr<Actor>(
        new Actor(m_models->getRefNum("spooky_tree.obj"))
        ));

    tree->moveTo(XMVectorSet(-7, 0, 6, 1));

    chekov->moveTo(XMVectorSet(0, 0, 7, 1));

    house->moveTo(XMVectorSet(-10, 0, 15, 1));
}


SceneDemo::~SceneDemo(void)
{
}

bool SceneDemo::update( float now, float timeSinceLastUpdate )
{
    duck->moveTo(XMVectorSet(-1.0f, 20.2f, 10.0f, 0));
    duck->setRollPitchYaw(-XM_PIDIV2 - XM_PIDIV4, 0);

    torus->moveTo(XMVectorSet(0, 1, 3, 1));

    return Scene::update(now, timeSinceLastUpdate);
}
