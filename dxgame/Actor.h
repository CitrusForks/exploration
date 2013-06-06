#pragma once

#include <directxmath.h>
#include <functional>


__declspec(align(16)) class Actor
{
private:
    DirectX::XMVECTOR m_position;
    //DirectX::XMVECTOR m_heading, m_upHint;
    DirectX::XMMATRIX m_correction; // a constant correction to be applied first to the world matrix just in case--mostly because our "art assets" are actually "jacked up downloads from the free 3d model sites"
    DirectX::XMMATRIX m_world;

    float m_pitch, m_yaw;

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
    //void setHeading(DirectX::FXMVECTOR heading);

    DirectX::XMVECTOR getHeading();
    //void estimatePitchYaw(float &out_pitch, float &out_yaw);
    void getPitchYaw(float &out_pitch, float &out_yaw) { out_pitch = m_pitch; out_yaw = m_yaw; }

    Actor(int modelRefNum);
    ~Actor(void);
};

