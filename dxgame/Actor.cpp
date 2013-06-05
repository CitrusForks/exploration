#include "stdafx.h"
#include "Actor.h"
#include <functional>
#include <DirectXMath.h>
#include <math.h>
#include <limits>
#include "dxmathprint.h"

using namespace DirectX;
using namespace std;

Actor::Actor(int modelRefNum) : m_modelRefNum(modelRefNum)
{
    m_heading = XMVectorSet(0, 0, 1, 0);
    m_position = XMVectorSet(0, 0, 0, 0);
    updateWorldMatrix();
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
    if (fabs(XMVector3Dot(up, m_heading).m128_f32[0]) > (1 - numeric_limits<float>::epsilon()) )
    {
        up  = XMVectorSet(0, 0, 1, 0);
    }

    // cobble together a matrix to rotate the object so it faces along m_heading
    // we're really tranforming the vertices to the vector space represented by m_heading so this isn't hard
    // m_heading had better be normalized
    XMFLOAT4X4 rotation;

    XMStoreFloat4x4(&rotation, XMMatrixIdentity());

    XMStoreFloat4((XMFLOAT4*) rotation.m[2], m_heading); // this column is the vector component along the +Z axis
    XMVECTOR x_component = XMVector3Normalize(XMVector3Cross(m_heading, up) * (-1)); // we're left-handed, hence multiply by -1 .___.
    XMStoreFloat4((XMFLOAT4*) rotation.m[0], x_component); // along the +X axis
    XMStoreFloat4((XMFLOAT4*) rotation.m[1], /*XMVector3Normalize*/(XMVector3Cross(m_heading, x_component))); // along the +Y axis, same regardless of handedness

    // however, the above writes to rows rather than columns so a transpose is required
    m_world = XMMatrixTranspose(XMLoadFloat4x4(&rotation)) * XMMatrixTranslationFromVector(m_position);

    cout << XMVectorSet(0,0,1,0) << "  -->  " << XMVector3TransformNormal(XMVectorSet(0,0,1,0), XMMatrixTranspose(XMLoadFloat4x4(&rotation))) << endl;
}

void Actor::setPitchYaw( float pitch, float yaw )
{
    XMVECTOR quato = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0);
    m_heading = XMVector3Normalize(XMVector3Rotate(XMVectorSet(0, 0, 1, 0), quato));
    updateWorldMatrix();
    //m_world = XMMatrixRotationQuaternion(quato) * XMMatrixTranslationFromVector(m_position);
}

void Actor::setHeading( DirectX::FXMVECTOR heading )
{
    m_heading = heading;
    updateWorldMatrix();
}


