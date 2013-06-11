#pragma once

#include <memory>

#include "Scene.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "d3dclass.h"

class SceneDemo :
    public Scene
{
public:
    SceneDemo(D3DClass &d3d, shared_ptr<ModelManager> models = nullptr, shared_ptr<TextureManager> textures = nullptr);
    ~SceneDemo(void);

    virtual bool update( float now, float timeSinceLastUpdate, FirstPerson &FPCamera );


private:
    shared_ptr<Actor> duck, chekov, tree, torus, house;

};

