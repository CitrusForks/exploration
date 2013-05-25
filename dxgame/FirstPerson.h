#ifndef FIRSTPERSON_H
#define FIRSTPERSON_H

#include <xnamath.h>
#include "Input.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h> // actually included already but I like listing all the headers required by a file...

class FirstPerson
{
private:
    XMFLOAT4 m_position;
    float m_height;
    float m_heading; // aka yaw, perhaps
    float m_pitch; // looking up at the ceiling, are we?

    int m_mouse_x, m_mouse_y;
public:
    FirstPerson(void);
    ~FirstPerson(void);

    void perFrameUpdate(double timeElapsed, Input &input);
    XMMATRIX getViewMatrix(); 
    XMVECTOR getPosition();
    XMVECTOR getEyePosition();
    void setPosition(FXMVECTOR to);

    static float movementSpeed;
    static float rotationSpeed;
};

#endif