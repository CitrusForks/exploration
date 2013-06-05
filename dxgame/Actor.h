#pragma once

#include <directxmath.h>
#include <functional>


__declspec(align(16)) class Actor
{
private:
    DirectX::XMVECTOR m_position;
    DirectX::XMVECTOR m_heading;
    DirectX::XMMATRIX m_world;

    int m_modelRefNum;
    
    // perhaps something like this, not final:
    int m_animation;
    float m_animationStartTime;

    void updateWorldMatrix();

public:

    bool render(std::function<bool(DirectX::CXMMATRIX world, int modelRefNum)> &renderFunc);

    void moveTo(DirectX::FXMVECTOR to);
    void move(DirectX::FXMVECTOR );

    void setPitchYaw(float pitch, float yaw);
    void setHeading(DirectX::FXMVECTOR heading);

    Actor(int modelRefNum);
    ~Actor(void);
};

