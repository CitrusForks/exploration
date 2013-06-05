#ifndef LIGHTSANDSHADOWS_H
#define LIGHTSANDSHADOWS_H

#include "cpp_hlsl_defs.h"
#include "directxmath.h"
#include "ShadowBuffer.h"
#include <vector>
#include <functional>

class FirstPerson;

#include "Light.h"
#include "d3dclass.h"
#include "ModelManager.h"
#include "vanillashaderclass.h"

// this class holds data on spotlights, directional lights, and the shadows they cast
__declspec(align(16)) class LightsAndShadows
{
private:
    DirectX::XMVECTOR lightDirection; // direction of directional light (sunlight/moonlight/whatever celestial object illuminates your world or whatnot)

    VanillaShaderClass shadowShaders;

    std::vector<Light> lights;
    vector<ShadowBuffer> shadows;

    DirectX::XMFLOAT4 sunlight;
    DirectX::XMFLOAT4 blue;



public:

    LightsAndShadows( D3DClass &d3d, HWND window);
    ~LightsAndShadows(void);

    // un-copy and un-assign
    LightsAndShadows(LightsAndShadows &no);
    LightsAndShadows &operator = (LightsAndShadows & nope); 
   

    void setFlashlight(FirstPerson &FPCamera, float beamHalfAngle = (float)M_PI * 15.0f/180.f, int whichLight = 0 );
    void setSpotlight( int which, DirectX::FXMVECTOR position, DirectX::FXMVECTOR direction, float beamHalfAngle = (float)M_PI * 15.0f/180.f, float constantAttenuation = 0.75f, float linearAttenuation = 0.0f, float quadraticAttenuation = 0.05f);

    void pointMoonlight(DirectX::FXMVECTOR newDirection, FirstPerson &FPCamera);

    // this method expects a function object — probably from a lambda — that can draw the current scene given the supplied shader and matrices
    bool renderShadowMaps( D3DClass &d3d, 
        std::function<bool(VanillaShaderClass &shader, DirectX::CXMMATRIX view, DirectX::CXMMATRIX projection, std::vector<Light> &lights)> doRenderCalls
        );

    void updateGPU(ID3D11DeviceContext *devCtx, VanillaShaderClass &shader, float time, DirectX::FXMVECTOR eyePosition);
    void setShadowsAsViewResources(D3DClass &d3d);

    std::vector<Light> &getLights() { return lights; }
    std::vector<ShadowBuffer> &getShadows() { return shadows; }
};

#endif
