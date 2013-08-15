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

Actor::Actor(int modelRefNum, DirectX::CXMMATRIX correction /* = XMMatrixIdentity() */) : m_modelRefNum(modelRefNum), m_lastRoll(0.0f), m_lastPitch(0.0f), m_lastYaw(0.0f), m_animation(-1), m_animationTime(-1)
{
    m_correction = correction;
    m_position = XMVectorSet(0, 0, 0, 0);
    m_rotation = XMQuaternionIdentity();
    updateWorldMatrix();
}

void Actor::init(int modelRefNum, DirectX::CXMMATRIX correction /* = DirectX::XMMatrixIdentity */)
{
    m_animationTime = -1;
    m_animation = -1;
    m_modelRefNum = modelRefNum;
    m_lastRoll = m_lastYaw = m_lastPitch = 0.0f;
    m_correction = correction;
    m_position = XMVectorSet(0, 0, 0, 0);
    m_rotation = XMQuaternionIdentity();
    m_slerpFinishTime = m_slerpStartTime = -1;
    updateWorldMatrix();
}

Actor::Actor()
{
    m_modelRefNum = -1;
}


Actor::~Actor(void)
{
}

// accepts a function that renders a model, calls it its world tranform matrix and its model refnum
bool Actor::render( renderFunc_t &renderFunc )
{
    if (m_modelRefNum == -1) Errors::Cry("Attempt to render uninitialized Actor object");
    return renderFunc(m_world, m_modelRefNum, m_animationTime);
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


void Actor::setPitchYawRoll( float pitch, float yaw, float roll /*= 0*/ )
{
    m_rotation = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
    updateWorldMatrix();
}


bool Actor::update(double now, double timeSinceLastUpdate)
{ 
    if (m_slerpFinishTime > now)
    {
        m_rotation = XMQuaternionSlerp(m_slerpFrom, m_slerpTo, (float)  (now - m_slerpStartTime) / (m_slerpFinishTime - m_slerpStartTime) );
    }

    m_animationTime = now; // XXX garbage for testing

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


