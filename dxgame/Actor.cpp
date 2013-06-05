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

Actor::Actor(int modelRefNum) : m_modelRefNum(modelRefNum)
{
    m_heading = XMVectorSet(0, 0, 1, 0);
    m_position = XMVectorSet(0, 0, 0, 0);
    m_upHint = XMVectorSet(0,1,0,0);
    updateWorldMatrix();

#if 0
    if (!arrrg) 
    {
        cout << "from: " << XMVectorSet(1,0,0,0) << "  to: " << XMVector3TransformNormal(XMVectorSet(1,0,0,0), m_world) << endl;
        arrrg = true;
    }
#endif // 0
}


Actor::~Actor(void)
{
}

bool Actor::render( std::function<bool(DirectX::CXMMATRIX world, int modelRefNum)> &renderFunc)
{
    return renderFunc(m_world, m_modelRefNum);
}

void Actor::moveTo( DirectX::FXMVECTOR to )
{
    m_position =to;
    updateWorldMatrix();
}

void Actor::updateWorldMatrix()
{
    XMVECTOR up = XMVectorSet(0, 1, 0, 0); 
    if (fabs(XMVector3Dot(up, m_heading).m128_f32[0]) > (1.0f - 100*numeric_limits<float>::epsilon()) )
    {
        up  = XMVectorSet(0, 0, -1, 0); // up and m_heading can't be parallel; this should still give us a good +X vector at least?
    }

    // cobble together a matrix to rotate the object so it faces along m_heading
    // we're really tranforming the vertices to the vector space represented by m_heading so this isn't hard
    // m_heading had better be normalized
    XMFLOAT4X4 rotation;

    XMStoreFloat4x4(&rotation, XMMatrixIdentity());

    XMStoreFloat4((XMFLOAT4*) rotation.m[2], m_heading); // this column is the vector component along the +Z axis
    XMVECTOR x_component = XMVector3Normalize(XMVector3Cross(up, m_heading)); 
    XMStoreFloat4((XMFLOAT4*) rotation.m[0], x_component); // along the +X axis
    XMStoreFloat4((XMFLOAT4*) rotation.m[1], /*XMVector3Normalize*/(XMVector3Cross(m_heading, x_component))); // along the +Y axis, same regardless of handedness

    m_world = XMLoadFloat4x4(&rotation) * XMMatrixTranslationFromVector(m_position);
}

void Actor::setPitchYaw( float pitch, float yaw )
{
    XMVECTOR quato = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0);
    m_heading = XMVector3Normalize(XMVector3Rotate(XMVectorSet(0, 0, 1, 0), quato));
    if (arrrg) updateWorldMatrix();
    else m_world = XMMatrixRotationQuaternion(quato) * XMMatrixTranslationFromVector(m_position);

    arrrg = !arrrg;
}

void Actor::setHeading( DirectX::FXMVECTOR heading )
{
    m_heading = heading;
    updateWorldMatrix();
}


