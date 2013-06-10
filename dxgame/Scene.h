#pragma once

#include <vector>
#include <memory>
#include "Actor.h"

// a scene just contains actors
// ...perhaps an additional Lua script?
class Scene
{
public:

    bool render(std::function<bool(DirectX::CXMMATRIX world, int modelRefNum)> &renderFunc); // render all actors

    virtual bool update(float now, float timeSinceLastUpdate); 

    int enters(shared_ptr<Actor> actor); // adds an actor, named with theater term to be precious
    void exits(int actor); // I hope you saved that reference from the enters() call...

    Scene(void);
    virtual ~Scene(void);

private:
    std::vector <shared_ptr<Actor>> m_actors;

};

