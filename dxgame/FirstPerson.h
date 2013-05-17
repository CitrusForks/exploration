#ifndef FIRSTPERSON_H
#define FIRSTPERSON_H

#include <xnamath.h>
#include <dinput.h>
#include "inputclass.h"

class FirstPerson
{
private:
    XMFLOAT4 m_position;
    float m_heading; // aka yaw, perhaps
    float m_pitch; // looking up at the ceiling, are we?


public:
    FirstPerson(void);
    ~FirstPerson(void);

    void perFrameUpdate(double timeElapsed, InputClass &input);
    XMMATRIX getViewMatrix(); 
    XMVECTOR getPosition();
    void setPosition(CXMVECTOR to);

    static float movementSpeed;
    static float rotationSpeed;
};

#endif