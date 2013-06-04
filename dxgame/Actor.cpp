#include "stdafx.h"
#include "Actor.h"
#include <functional>


Actor::Actor(void)
{
}


Actor::~Actor(void)
{
}

bool Actor::render( std::function<bool(DirectX::CXMMATRIX world, int modelRefNum)> &renderFunc)
{
    return renderFunc(XMLoadFloat4x4(&m_world), m_modelRefNum);
}
