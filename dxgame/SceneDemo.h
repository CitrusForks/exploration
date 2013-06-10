#pragma once

#include <memory>

#include "Scene.h"
#include "ModelManager.h"
#include "TextureManager.h"

class SceneDemo :
    public Scene
{
public:
    SceneDemo(shared_ptr<ModelManager> models = nullptr, shared_ptr<TextureManager> textures = nullptr);
    ~SceneDemo(void);

private:
    shared_ptr<ModelManager> m_models;
    shared_ptr<TextureManager> m_textures;
};

