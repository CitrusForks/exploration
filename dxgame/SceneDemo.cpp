#include "stdafx.h"
#include "SceneDemo.h"
#include <memory>
#include "TextureManager.h"
#include "ModelManager.h"


SceneDemo::SceneDemo(shared_ptr<ModelManager> models, shared_ptr<TextureManager> textures)
{
    m_models = models ? models : make_shared<ModelManager>();
    m_textures = textures ? textures : make_shared<TextureManager>();

    (*m_models)["Chekov.obj"];

    (*m_models)["duck.obj"]; // retrieving an unloaded model triggers a load from disk

    (*m_models)["floor.obj"];

    (*m_models)["torus.obj"];

    (*m_models)["LPBuildX13r_3ds.3ds"];

    (*m_models)["spooky_tree.obj"];
    // WARNING, pointers returned by models["whatever"] may be invalid later if a model is loaded and the internal vector is reallocated. Use refnums to store an index for longterm fast lookup!

}


SceneDemo::~SceneDemo(void)
{
}
