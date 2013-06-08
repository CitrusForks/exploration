#pragma once

#include <vector>
#include "Actor.h"

// a scene just contains actors
// ...perhaps an additional Lua script?
class Scene
{
    std::vector <Actor *> m_actors;

public:

    bool render(std::function<bool(DirectX::CXMMATRIX world, int modelRefNum)> &renderFunc); // render all actors

    virtual bool update(float now, float timeSinceLastUpdate); 

    Scene(void);
    virtual ~Scene(void);
};

