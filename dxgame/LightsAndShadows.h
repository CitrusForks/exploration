#ifndef LIGHTSANDSHADOWS_H
#define LIGHTSANDSHADOWS_H

#include "cpp_hlsl_defs.h"
#include "directxmath.h"
#include "ShadowBuffer.h"
#include <vector>

class FirstPerson;
class VanillaShaderClass;

// this struct is mostly for spotlights but can be used to store data for other types of lights as well
struct Light
{
    DirectX::XMFLOAT4 position; // world space
    DirectX::XMFLOAT3 direction;

    float halfAngle; // angle between axis and generatrix (if you pretend the spotlight is a perfect cone)

    // { cos(halfAngle), constant attenuation, linear att., quadratic att. }:
    float cosHalfAngle;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    // these used to be an XMFLOAT4 but I wanted to give them names and XMFLOAT4 can't be in a union due to a non-trivial constructor

    DirectX::XMFLOAT4X4 projection; // View*Projection matrices ("VP") for all the lights, >>>including the directional light in the final position x2 LOD<<<

    DirectX::XMFLOAT4 color; // colored lights can hypnotize?

    bool enabled;

    Light(void) : position(0,0,0,0), direction(0,-1,0), halfAngle((float)M_PI*(18.0f/180.0f)), cosHalfAngle(cos(halfAngle)), constantAttenuation(0.75f), linearAttenuation(0),
        quadraticAttenuation(0.05f), color(1,1,1,1), enabled(false)
    {
        updateProjection();
    }

    void setHalfAngle(float ha)
    {
        halfAngle = ha;
        cosHalfAngle = cos(ha);
        updateProjection();
    }

    void updateProjection()
    {
        DirectX::XMMATRIX viewProj = DirectX::XMMatrixLookToLH(XMLoadFloat4(&position), DirectX::XMLoadFloat3(&direction), DirectX::XMVectorSet(0, 1, 0, 0)) * 
            DirectX::XMMatrixPerspectiveFovLH(halfAngle*2, 1.0f /* shadowmap is a square at any resolution */, 0.1f, 1000.0f); // the 1000 far plane should perhaps be configurable?
        DirectX::XMStoreFloat4x4(&projection, viewProj);
    }

    void move(DirectX::FXMVECTOR newPosition, DirectX::FXMVECTOR newDirection /* no relation to the band */)
    {
        DirectX::XMStoreFloat4(&position, newPosition);
        DirectX::XMStoreFloat3(&direction, newDirection);
        updateProjection();
    }
};


class LightsAndShadows
{
private:
    std::vector<Light> lights;
    vector<ShadowBuffer> shadows;

    DirectX::XMVECTOR lightDirection; // direction of directional light (sunlight/moonlight/whatever celestial object illuminates your world or whatnot)

    DirectX::XMFLOAT4 sunlight;
    DirectX::XMFLOAT4 blue;

public:
    LightsAndShadows(ID3D11Device *device, ID3D11DeviceContext *devCtx);
    ~LightsAndShadows(void);
    void setFlashlight( ID3D11Device *device, ID3D11DeviceContext *devCtx, FirstPerson &FPCamera, int whichLight = 0 );
    void setSpotlight( int which, DirectX::FXMVECTOR position, DirectX::FXMVECTOR direction, float beamHalfAngle = (float)M_PI * 15.0f/180.f, float constantAttenuation = 0.75f, float linearAttenuation = 0.0f, float quadraticAttenuation = 0.05f);
    void pushToGPU(ID3D11DeviceContext *devCtx, VanillaShaderClass &shader, float time, DirectX::FXMVECTOR eyePosition);
};

#endif