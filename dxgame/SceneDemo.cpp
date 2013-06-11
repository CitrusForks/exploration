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

    (*m_models)["duck.obj"]; // retrieving an unloaded model triggers a load from disk

    // WARNING, pointers returned by models["whatever"] may be invalid later if a model is loaded and the internal vector is reallocated. Use refnums to store an index for longterm fast lookup!
    enters(chekov = shared_ptr<Actor>(
        new Actor(m_models->getRefNum("Chekov.obj"), XMMatrixScaling(0.53f, 0.53f, 0.53f))
        ));

    enters(duck = shared_ptr<Actor>(
        new Actor(m_models->getRefNum("duck.obj"), XMMatrixTranslation(0, -0.2f, 0))
        ));

    enters(torus = shared_ptr<Actor>(
        new Actor(m_models->getRefNum("torus.obj"))
        ));

    enters(tree = shared_ptr<Actor>(
        new Actor(m_models->getRefNum("spooky_tree.obj"))
        ));

    enters(shared_ptr<Actor>(
        new Actor(m_models->getRefNum("floor.obj"))
        ));
#if 1

    enters(house = shared_ptr<Actor>(
        new Actor(m_models->getRefNum("LPBuildX13r_3ds.3ds"), XMMatrixRotationAxis(XMVectorSet(1.0f,0,0,0), (float)M_PI_2) * XMMatrixScaling(0.15f, 0.15f, 0.15f) * XMMatrixTranslation(0.0f, 4.5001f, 0))
        ));

    house->moveTo(XMVectorSet(-10, 0, 15, 1));

#endif
    tree->moveTo(XMVectorSet(-7, 0, 6, 1));

    chekov->moveTo(XMVectorSet(0, 0, 7, 1));

}


SceneDemo::~SceneDemo(void)
{
}

bool SceneDemo::update( float now, float timeSinceLastUpdate, FirstPerson &FPCamera )
{
    duck->moveTo(XMVectorSet(-1.0f, -0.2f + sin(now*2)/8, 10.0f, 0));
    //duck->setRollPitchYaw(-XM_PIDIV2 - XM_PIDIV4, 0);

    torus->moveTo(XMVectorSet(0, 1, 3, 1));
    torus->setRollPitchYaw(now, 0);

    m_lighting->setFlashlight(FPCamera, XMConvertToRadians(25.0f/2));
    m_lighting->pointMoonlight(XMVector3Normalize(XMVectorSet(0.1f,  -0.2f, 1.0f, 0.0f)), FPCamera);

    return Scene::update(now, timeSinceLastUpdate, FPCamera);
}
