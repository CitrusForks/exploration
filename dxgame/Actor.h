#pragma once

#include <directxmath.h>
#include <functional>


__declspec(align(16)) class Actor
{
private:
    DirectX::XMVECTOR m_position;
    DirectX::XMMATRIX m_correction; // a constant correction to be applied first to the world matrix just in case--mostly because our "art assets" are actually "jacked up downloads from the free 3d model sites"
    DirectX::XMMATRIX m_world;

    DirectX::XMVECTOR m_rotation;

    DirectX::XMVECTOR m_slerpFrom, m_slerpTo; // for SLERPies

    float m_slerpStartTime, m_slerpFinishTime;

    float m_lastYaw, m_lastRoll, m_lastPitch; // just for convenience

    int m_modelRefNum;
    
    // perhaps something like this, not final:
    int m_animation; // ???
    float m_animationStartTime; // ???

    // for internal use, updates world matrix:
    void updateWorldMatrix();

public:

    bool render(std::function<bool(DirectX::CXMMATRIX world, int modelRefNum)> &renderFunc);

    void moveTo(DirectX::FXMVECTOR to);
    void move(DirectX::FXMVECTOR delta);

    void setRollPitchYaw(float pitch, float yaw, float roll = 0);

    void slerp(float now, float finish, float pitch, float yaw, float roll = 0);
    void slerp(float now, float finish, DirectX::FXMVECTOR quaternion);

    DirectX::XMVECTOR getQuaternion() { return m_rotation; }
    void getRollPitchYaw(float &out_pitch, float &out_yaw, float &out_roll) { out_pitch = m_lastPitch; out_yaw = m_lastYaw; out_roll = m_lastRoll; } 

    // the default implementation only updates SLERP:
    virtual bool update(float now, float timeSinceLastUpdate); // return false as a hint to have yourself deleted from world.

    Actor(int modelRefNum);
    ~Actor(void);
};

