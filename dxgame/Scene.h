#pragma once

#include <vector>
#include "Actor.h"

// a scene just contains actors
// ...perhaps an additional Lua script?
class Scene
{
    std::vector <Actor *> actors;

public:

    bool render(std::function<bool(DirectX::CXMMATRIX world, int modelRefNum)> &renderFunc); // render all actors


    Scene(void);
    ~Scene(void);
};

