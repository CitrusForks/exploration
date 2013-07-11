#pragma once

#include <directxmath.h>
#include <functional>
#include "SSE2Aligned.h"


__declspec(align(16)) class Actor 
    : public SSE2Aligned
{
private:
    DirectX::XMMATRIX m_correction; // a constant correction to be applied first to the world matrix just in case--mostly because our "art assets" are actually "jacked up downloads from the free 3d model sites"
    DirectX::XMMATRIX m_world;

    DirectX::XMVECTOR m_position;

    DirectX::XMVECTOR m_rotation;

    DirectX::XMVECTOR m_slerpFrom, m_slerpTo; // for SLERPies

    float m_slerpStartTime, m_slerpFinishTime;

    float m_lastYaw, m_lastRoll, m_lastPitch; // just for convenience

    int m_modelRefNum;
  
    int m_sceneID; // or any extra id or other opaque integer label slapped onto the object by external agents

    // perhaps something like this, not final:
    int m_animation; // ???
    float m_animationStartTime; // ???

    // for internal use, updates world matrix:
    void updateWorldMatrix();

public:

    typedef std::function<bool(DirectX::CXMMATRIX world, int modelRefNum)> renderFunc_t;
    bool render(renderFunc_t &renderFunc);

    void moveTo(DirectX::FXMVECTOR to);
    void move(DirectX::FXMVECTOR delta);

    void setPitchYawRoll(float pitch, float yaw, float roll = 0);

    void slerp(float now, float finish, float pitch, float yaw, float roll = 0);
    void slerp(float now, float finish, DirectX::FXMVECTOR quaternion);

    DirectX::XMVECTOR getQuaternion() { return m_rotation; }
    void getRollPitchYaw(float &out_pitch, float &out_yaw, float &out_roll) { out_pitch = m_lastPitch; out_yaw = m_lastYaw; out_roll = m_lastRoll; } 

    // the default implementation only updates SLERP:
    virtual bool update(float now, float timeSinceLastUpdate); // return false as a hint to have yourself deleted from world.

    // set m_sceneID; only Scene is likely to have that information, let it be set apart from construction
    void setID(int id) { m_sceneID = id; }

    Actor();
    void init(int modelRefNum, DirectX::CXMMATRIX correction = DirectX::XMMatrixIdentity());
    Actor(int modelRefNum, DirectX::CXMMATRIX correction = DirectX::XMMatrixIdentity());
    virtual ~Actor(void);

};