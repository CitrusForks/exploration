#pragma once

#include <directxmath.h>
#include <functional>


class Actor
{
private:
    DirectX::XMFLOAT4 m_position;
    DirectX::XMFLOAT4 m_heading;
    DirectX::XMFLOAT4X4 m_world;

    int m_modelRefNum;
    
    // perhaps something like this, not final:
    int m_animation;
    float m_animationStartTime;

public:

    bool render(std::function<bool(DirectX::CXMMATRIX world, int modelRefNum)> &renderFunc);

    Actor(void);
    ~Actor(void);
};

