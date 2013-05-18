#include "StdAfx.h"
#include "FirstPerson.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>


float FirstPerson::movementSpeed = 2.0f; // in m/s ... 1 unit is 1 meter is the convention in this project; 2.0m/s is quite a hurried walk
float FirstPerson::rotationSpeed = (float)(140.0/360.0 * M_PI * 2);


FirstPerson::FirstPerson(void) : m_pitch(0.0f), m_heading(0.0f), m_position(0.0f, 0.0f, 0.0f, 1.0f)
{
}


FirstPerson::~FirstPerson(void)
{
}


// move based on input
void FirstPerson::perFrameUpdate(double timeElapsed, InputClass &input)
{
    XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);


    // turn with arrow keys like it's 1995
    // joystick turning can be added here though
    if (input.IsPressed(DIK_RIGHT) || input.IsPressed(DIK_RIGHTARROW))
    {
        m_heading += (float) (rotationSpeed * timeElapsed);
#ifdef DEBUG_SPAM
        XMFLOAT4 tmp;
        XMStoreFloat4(&tmp, XMVector3Rotate(forward, XMQuaternionRotationRollPitchYaw(0.0f, m_heading, 0.0f)));
        std::cout << m_heading << ", [" << tmp.x << ", " << tmp.y << ", " << tmp.z << ", " << tmp.w << "]" << std::endl;
#endif

    } else if (input.IsPressed(DIK_LEFT) || input.IsPressed(DIK_LEFTARROW))
    {
        m_heading -= (float) (rotationSpeed * timeElapsed);
    }


    // back and forward walk
    if (input.IsPressed(DIK_UP) || input.IsPressed(DIK_UPARROW) || input.IsPressed(DIK_W) || input.IsPressed(DIK_S) || input.IsPressed(DIK_DOWN) || input.IsPressed(DIK_DOWNARROW))
    {
        int sign = (input.IsPressed(DIK_S) || input.IsPressed(DIK_DOWN) || input.IsPressed(DIK_DOWNARROW)) ? -1 : 1; // XXX refactor me

        XMStoreFloat4(&m_position, 
            XMLoadFloat4(&m_position) + XMVector3Rotate(forward, XMQuaternionRotationRollPitchYaw(0.0f, m_heading, 0.0f)) * (float)(movementSpeed * timeElapsed * sign) 
           );

    }

    // strafe 
    if (input.IsPressed(DIK_A) || input.IsPressed(DIK_D))
    {
        int sign = input.IsPressed(DIK_A) ? -1 : 1;

        XMStoreFloat4(&m_position, 
            XMLoadFloat4(&m_position) + XMVector3Rotate(forward, XMQuaternionRotationRollPitchYaw(0.0f, m_heading + (float)M_PI_2, 0.0f)) * (float)(movementSpeed * timeElapsed * sign) 
           );
    }
}


// matrix for worldspace->viewspace
XMMATRIX FirstPerson::getViewMatrix()
{
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    XMVECTOR height = XMVectorSet(0.0f, 1.8f, 0.0f, 0.0f); // a 1.8m tall person is OK, perhaps on the tall side

    return XMMatrixLookToLH(XMLoadFloat4(&m_position) + height, XMVector3Rotate(forward, XMQuaternionRotationRollPitchYaw(m_pitch, m_heading, 0.0f)), up);
}

// just the position
XMVECTOR FirstPerson::getPosition()
{
    return XMLoadFloat4(&m_position);
}

// currently unused...
void FirstPerson::setPosition(CXMVECTOR to)
{
    XMStoreFloat4(&m_position, to);
}
