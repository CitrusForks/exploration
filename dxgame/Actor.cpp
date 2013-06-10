#include "stdafx.h"
#include "Actor.h"
#include <functional>
#include <DirectXMath.h>
#include <math.h>
#include <limits>
#include "dxmathprint.h"

using namespace DirectX;
using namespace std;

static bool arrrg = false;

Actor::Actor(int modelRefNum, DirectX::CXMMATRIX correction /* = XMMatrixIdentity() */) : m_modelRefNum(modelRefNum), m_lastRoll(0.0f), m_lastPitch(0.0f), m_lastYaw(0.0f)
{
    m_correction = correction;
    m_position = XMVectorSet(0, 0, 0, 0);
    m_rotation = XMQuaternionIdentity();
    updateWorldMatrix();

}


Actor::~Actor(void)
{
}


bool Actor::render( std::function<bool(DirectX::CXMMATRIX world, int modelRefNum)> renderFunc )
{
    return renderFunc(m_world, m_modelRefNum);
}


void Actor::moveTo( DirectX::FXMVECTOR to )
{
    m_position = to;
    updateWorldMatrix();
}


void Actor::updateWorldMatrix()
{
    m_world = m_correction * XMMatrixRotationQuaternion(m_rotation) * XMMatrixTranslationFromVector(m_position);
}


void Actor::setRollPitchYaw( float pitch, float yaw, float roll /*= 0*/ )
{
    m_rotation = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
}


bool Actor::update(float now, float timeSinceLastUpdate)
{ 
    if (m_slerpFinishTime > now)
    {
        m_rotation = XMQuaternionSlerp(m_slerpFrom, m_slerpTo,  (now - m_slerpStartTime) / (m_slerpFinishTime - m_slerpStartTime) );
    }

    return true;
}

void Actor::slerp( float now, float finish, float pitch, float yaw, float roll /*= 0*/ )
{
    slerp(now, finish, XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));
}

void Actor::slerp( float now, float finish, FXMVECTOR quaternion )
{
    m_slerpFrom = m_rotation;
    m_slerpTo = quaternion;

    m_slerpStartTime = now;
    m_slerpFinishTime = finish;
}


