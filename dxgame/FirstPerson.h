#ifndef FIRSTPERSON_H
#define FIRSTPERSON_H

#include <directxmath.h>
#include "Input.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h> // actually included already but I like listing all the headers required by a file...

class FirstPerson
{
private:
    DirectX::XMFLOAT4 m_position;
    float m_height;
    float m_heading; // aka yaw, perhaps
    float m_pitch; // looking up at the ceiling, are we?

    bool m_crouching; // need to cache this

    int m_mouse_x, m_mouse_y;
public:
    FirstPerson(void);
    ~FirstPerson(void);

    void perFrameUpdate(double timeElapsed, Input &input);

    DirectX::XMVECTOR getForwardVector();

    DirectX::XMMATRIX getViewMatrix(); 
    DirectX::XMVECTOR getPosition();
    DirectX::XMVECTOR getEyePosition();
    void setPosition(DirectX::FXMVECTOR to);

    float getMoveSpeed(Input &input); // with modifiers

    static float movementSpeed;
    static float rotationSpeed;
};

#endif