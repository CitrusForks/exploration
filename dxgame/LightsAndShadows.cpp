#include "stdafx.h"
#include "LightsAndShadows.h"
#include "directxmath.h"
#include <d3d11.h>
#include <math.h>

using namespace DirectX;

LightsAndShadows::LightsAndShadows(ID3D11Device *device, ID3D11DeviceContext *devCtx) : sunlight( 255.0f / 255.0f, 255.0f / 255.0f, 251.0f / 255.0f, 1.0f ), blue(0, 0, 1.0f, 1.0f )
{
    lights.resize(NUM_SPOTLIGHTS+2); // spotlights + 2 spots for directional lights

    float defaultHalfAngles[NUM_SPOTLIGHTS] = { (float)M_PI * 15.0f/180.f, (float)M_PI * 25.0f/180.f, 0.0f, 0.0f }; // XXX move to default scene
    for (int i; i < NUM_SPOTLIGHTS; ++i) lights[i].halfAngle = defaultHalfAngles[i];

    // shadow maps! 
    shadows.resize(NUM_SPOTLIGHTS + 2); // +2 for directional light x2 LOD
    for (unsigned i = 0; i < shadows.size()-1; ++i)
    {
        shadows[i].init(device, devCtx);
    }

    // shadow maps for directional lights:
    shadows[shadows.size()-2].init(device, devCtx, 8); // 4Kx4K texture... kinda hefty, this is for the sweeping 100mx100m view
    shadows[shadows.size()-1].init(device, devCtx, 4); // this is for the player's immediate surroundings

}


LightsAndShadows::~LightsAndShadows(void)
{
}





void LightsAndShadows::setFlashlight( ID3D11Device *device, ID3D11DeviceContext *devCtx, FirstPerson &FPCamera, int whichLight /*= 0 */ )
{
    XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    lights[whichLight].move(
        XMVector4Transform(FPCamera.getEyePosition(), XMMatrixTranslation(0.15f, -0.6f, 0.1f)), // hold flashlight lower... 
        XMVector3TransformNormal(forward, XMMatrixRotationAxis(XMVectorSet(1,0,0,0), (float)-M_PI/8)  *  XMMatrixTranspose(FPCamera.getViewMatrix())) // tilt it
        );
    // re: the above transpose; view matrix is the inverse of what we want to transform a vector extending from the eye and the 3x3 excerpt of the view matrix for a normal transform is orthogonal so the inverse is the transpose

    // lightPos[1] = XMFLOAT4(4.0f, 4.0f, 3.0f, 1.0f);

    // XMVECTOR axis45deg = XMVectorSet(-0.7071067811865475f, -0.7071067811865475f, 0.0f, 0.0f); // normalized 45 degree angle vector
    // XMStoreFloat3(&(lightDir[1]), axis45deg);

    XMFLOAT4X4 lightProj[NUM_SPOTLIGHTS+2]; // View*Projection matrices ("VP") for all the lights, >>>including the directional light in the final position (x2 LOD)<<<

    for (int i = 0; i < numLights; ++i)
    {
    }

    XMVECTOR lightDirection = XMVector3Normalize(XMVectorSet(0.1f,  -0.2f, 1.0f, 0.0f)); // directional light

    // wide view directional shadow
    XMVECTOR skyCamPos = FPCamera.getEyePosition();
    skyCamPos = XMVectorFloor(skyCamPos + XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f) - lightDirection * 90);

    XMMATRIX directionalShadowOrtho = XMMatrixLookToLH(skyCamPos, 
        lightDirection, XMVectorSet(0, 0, 1, 0)) * XMMatrixOrthographicLH(100, 100, 0.1f, 1000); // view * orthographic projection, for directional light
    // N.B., "up" should be orthogonal to lightDirection, or at least not parallel because a crossproduct is performed

    XMStoreFloat4x4(lightProj+NUM_SPOTLIGHTS, directionalShadowOrtho);

    // high LOD view for directional shadow, focused on the player's vicinity
    skyCamPos = FPCamera.getEyePosition();
    skyCamPos = /* XMVectorFloor */(skyCamPos /* + XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f) */ - lightDirection * 90 + FPCamera.getForwardVector()*3.5);
    directionalShadowOrtho = XMMatrixLookToLH(skyCamPos, 
        lightDirection, XMVectorSet(0, 0, 1, 0)) * XMMatrixOrthographicLH(7, 7, 0.1f, 1000); // view * orthographic projection, for directional light

    XMStoreFloat4x4(lightProj+NUM_SPOTLIGHTS+1, directionalShadowOrtho);



    XMFLOAT4 sunlight( 255.0f / 255.0f, 255.0f / 255.0f, 251.0f / 255.0f, 1.0f ); // sun yellow orange
    XMFLOAT4 blue(0, 0, 1.0f, 1.0f );
}

void LightsAndShadows::pushToGPU( ID3D11DeviceContext *devCtx, VanillaShaderClass &shader, float time, FXMVECTOR eyePosition )
{
    shader.SetPSLights(devCtx, lightDirection, time, eyePosition, lights, blue, sunlight);
}

void LightsAndShadows::setSpotlight( int which, DirectX::FXMVECTOR position, DirectX::FXMVECTOR direction, float beamHalfAngle /*= (float)M_PI * 15.0f/180.f*/, float constantAttenuation /*= 0.75f*/, float linearAttenuation /*= 0.0f*/, float quadraticAttenuation /*= 0.05f*/ )
{
    assert(which < NUM_SPOTLIGHTS);

    XMStoreFloat4(&lights[which].position, position);
    XMStoreFloat3(&lights[which].direction, direction);
    lights[which].halfAngle = beamHalfAngle;
    lights[which].cosHalfAngle = cos(beamHalfAngle);

}
