#include "StdAfx.h"
#include "FirstPerson.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

using namespace std;
using namespace DirectX;

float FirstPerson::movementSpeed = 2.0f; // in m/s ... 1 unit is 1 meter is the convention in this project; 2.0m/s is quite a hurried walk... but walking feels slow
float FirstPerson::rotationSpeed = (float)(140.0/360.0 * M_PI * 2);


FirstPerson::FirstPerson(void) : m_pitch(0.0f), m_heading(0.0f), m_position(0.0f, 0.0f, 0.0f, 1.0f), m_height(1.8f), m_mouse_x(-1), m_mouse_y(-1), m_crouching(false)
{
    // 1.8m tall = 5'9"
}


FirstPerson::~FirstPerson(void)
{
}


float FirstPerson::getMoveSpeed( Input &input )
{
    float speed = movementSpeed;

    if (input.IsPressed(DIK_LSHIFT) || input.IsPressed(DIK_RSHIFT))
    {
        speed *= 3; // running
    }

    if (m_crouching) speed /= 2;

    return speed;
}


// move based on input
void FirstPerson::perFrameUpdate(double timeElapsed, Input &input)
{
    XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    XMVECTOR offset = XMVectorZero();

    m_crouching = input.IsPressed(DIK_LCONTROL);


    // turn with arrow keys like it's 1995
    // joystick turning can be added here though
    if (input.IsPressed(DIK_RIGHT) || input.IsPressed(DIK_RIGHTARROW))
    {
        m_heading += (float) (rotationSpeed * timeElapsed);
#ifdef DEBUG_SPAM
        XMFLOAT4 tmp;
        XMStoreFloat4(&tmp, XMVector3Rotate(forward, XMQuaternionRotationRollPitchYaw(0.0f, m_heading, 0.0f)));
        cout << m_heading << ", [" << tmp.x << ", " << tmp.y << ", " << tmp.z << ", " << tmp.w << "]" << endl;
#endif

    } else if (input.IsPressed(DIK_LEFT) || input.IsPressed(DIK_LEFTARROW))
    {
        m_heading -= (float) (rotationSpeed * timeElapsed);
    }

    bool moving = false; // because we can't do comparisons on XMVECTOR values directly; this is less verbose than digging through the vector

    // back and forward walk
    if (input.IsPressed(DIK_UP) || input.IsPressed(DIK_UPARROW) || input.IsPressed(DIK_W) || input.IsPressed(DIK_S) || input.IsPressed(DIK_DOWN) || input.IsPressed(DIK_DOWNARROW))
    {
        int sign = (input.IsPressed(DIK_S) || input.IsPressed(DIK_DOWN) || input.IsPressed(DIK_DOWNARROW)) ? -1 : 1; // XXX refactor me
        moving = true;

        offset += getForwardVector() * (float)sign;

    }

    // strafe 
    if (input.IsPressed(DIK_A) || input.IsPressed(DIK_D))
    {
        int sign = input.IsPressed(DIK_A) ? -1 : 1;
        moving = true;

        offset += XMVector3Rotate(forward, XMQuaternionRotationRollPitchYaw(0.0f, m_heading + (float)M_PI_2, 0.0f)) * (float)sign;

    }

    if (moving)
    {
        // normalize offset to avoid going faster when hitting forward+strafe :P
        XMStoreFloat4(&m_position, 
            XMLoadFloat4(&m_position) + XMVector3Normalize(offset)*(float)(getMoveSpeed(input) * timeElapsed)
            );
    }

    // mouse stuff?
    int x, y;
    input.GetMouseLocation(x, y);
    if (m_mouse_x == -1 && m_mouse_y == -1)
    {
        // first frame, don't go wild with crazy mouse inputs
    } else
    {
        m_heading += (float) (timeElapsed * (x - m_mouse_x) * rotationSpeed * (Options::intOptions["MouseSensitivity"] / 32.0f)); // x movement rotates
        
        m_pitch += (float) (timeElapsed * (y - m_mouse_y) * (Options::intOptions["MouseSensitivity"] / 32.0f)); // y movement looks up and down
        if (m_pitch < (float)-M_PI_2+0.001f) m_pitch = (float) -M_PI_2+0.001f; // clamp pitch
        if (m_pitch > (float)M_PI_2-0.001f) m_pitch = (float) M_PI_2-0.001f; 
    }
    m_mouse_x = x;
    m_mouse_y = y;
}


// matrix for worldspace->viewspace
XMMATRIX FirstPerson::getViewMatrix()
{
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    return XMMatrixLookToLH(getEyePosition(), XMVector3Rotate(forward, XMQuaternionRotationRollPitchYaw(m_pitch, m_heading, 0.0f)), up);
}

// just the position, at foot level
XMVECTOR FirstPerson::getPosition()
{
    return XMLoadFloat4(&m_position);
}


// at eye level...
XMVECTOR FirstPerson::getEyePosition()
{
    return XMLoadFloat4(&m_position) + XMVectorSet(0.0f, m_crouching ? m_height/1.5f : m_height, 0.0f, 0.0f); 
}



// currently unused...
void FirstPerson::setPosition(FXMVECTOR to)
{
    XMStoreFloat4(&m_position, to);
}


// heading in vector form
XMVECTOR FirstPerson::getForwardVector(bool includePitch)
{
    return XMVector3Normalize(XMVector3Rotate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMQuaternionRotationRollPitchYaw(includePitch ? m_pitch : 0.0f, m_heading, 0.0f))); // the quaternion is used just to prove to myself that I can use a quaternion
}


// point the camera somewhere, e.g., for initial setup
void FirstPerson::setYawPitch( float yaw, float pitch /*= 0.0f*/ )
{
    m_heading = yaw;
    m_pitch = pitch;
}

