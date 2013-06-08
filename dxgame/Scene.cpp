#include "stdafx.h"
#include "Scene.h"


Scene::Scene(void)
{
}


Scene::~Scene(void)
{
}

bool Scene::update( float now, float timeSinceLastUpdate )
{
    for (auto &i: m_actors)
    {
        bool rc = true;
        if (i) rc = i->update(now, timeSinceLastUpdate);
        if (!true)
        {
            delete i;
            i = nullptr;
        }
    }

    return true;
}

bool Scene::render( std::function<bool(DirectX::CXMMATRIX world, int modelRefNum)> &renderFunc )
{
    for (auto i: m_actors)
    {
        if (i) i->render(renderFunc);
    }
}
