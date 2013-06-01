#include "stdafx.h"
#include "LightsAndShadows.h"
#include "directxmath.h"
#include <d3d11.h>
#include <math.h>
#include <windef.h>
#include "d3dclass.h"
#include "ShadowBuffer.h"

using namespace DirectX;
using namespace std;

LightsAndShadows::LightsAndShadows(D3DClass &d3d, HWND window) : sunlight( 255.0f / 255.0f, 255.0f / 255.0f, 251.0f / 255.0f, 1.0f ), blue(0, 0, 1.0f, 1.0f )
{
    // shadowShaders contains a cut-down pixel shader to facilitate generating a shadow map from a depth buffer
    shadowShaders.InitializeShader(d3d.GetDevice(), window, L"light_vs.cso", L"shadow_ps.cso");

    lights.resize(NUM_SPOTLIGHTS+2); // spotlights + 2 spots for directional lights

    float defaultHalfAngles[NUM_SPOTLIGHTS] = { (float)M_PI * 15.0f/180.f, (float)M_PI * 25.0f/180.f, 0.0f, 0.0f }; // XXX move to default scene
    for (int i; i < NUM_SPOTLIGHTS; ++i) lights[i].halfAngle = defaultHalfAngles[i];

    // shadow maps! 
    shadows.resize(NUM_SPOTLIGHTS + 2); // +2 for directional light x2 LOD
    for (unsigned i = 0; i < shadows.size()-1; ++i)
    {
        shadows[i].init(d3d.GetDevice(), d3d.GetDeviceContext());
    }

    // shadow maps for directional lights:
    shadows[shadows.size()-2].init(d3d.GetDevice(), d3d.GetDeviceContext(), 8); // 4Kx4K texture... kinda hefty, this is for the sweeping 100mx100m view
    shadows[shadows.size()-1].init(d3d.GetDevice(), d3d.GetDeviceContext(), 4); // this is for the player's immediate surroundings

    lightDirection = XMVector3Normalize(XMVectorSet(0.1f,  -0.2f, 1.0f, 0.0f)); // directional light

    // default test light
    XMVECTOR axis45deg = XMVectorSet(-0.7071067811865475f, -0.7071067811865475f, 0.0f, 0.0f); // normalized 45 degree angle vector
    lights[1].move(XMVectorSet(4.0f, 4.0f, 3.0f, 1.0f), axis45deg);
}

// copy protection :P
LightsAndShadows::LightsAndShadows( LightsAndShadows &no )
{
    Errors::Cry("Program error! You cannot copy LightsAndShadows()!");
}

LightsAndShadows & LightsAndShadows::operator=( LightsAndShadows & nope )
{
    Errors::Cry("Program error! You cannot assign LightsAndShadows! Try std::shared_ptr<> to do whatever it is you're trying to accomplish.");
}

// this destructor mainly releases the shadow buffer
LightsAndShadows::~LightsAndShadows(void)
{
    for (auto i = shadows.begin(); i != shadows.end(); ++i)
    {
        i->release();
    }
}

// set a spotlight, index 0 by default, to act as the player's flashlight
// TODO: add a spotlight model with a mesh and add a spotlight to that instead
void LightsAndShadows::setFlashlight( FirstPerson &FPCamera, int whichLight /*= 0 */ )
{
    XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // left-handed forward vector for the player, just to make it obvious what this is
    lights[whichLight].move(
        XMVector4Transform(FPCamera.getEyePosition(), XMMatrixTranslation(0.15f, -0.6f, 0.1f)), // hold flashlight lower... 
        XMVector3TransformNormal(forward, XMMatrixRotationAxis(XMVectorSet(1,0,0,0), (float)-M_PI/8)  *  XMMatrixTranspose(FPCamera.getViewMatrix())) // tilt it a bit so the light doesn't look like a stupid circle
        );
    // re: the above transpose; view matrix is the inverse of what we want to transform a vector extending from the eye and the 3x3 excerpt of the view matrix for a normal transform is orthogonal so the inverse is the transpose
}

// this mainly calls the appropriate shader method to store data in the pixel shader's constant buffer for lights
void LightsAndShadows::pushToGPU( ID3D11DeviceContext *devCtx, VanillaShaderClass &shader, float time, FXMVECTOR eyePosition )
{
    shader.SetPSLights(devCtx, lightDirection, time, eyePosition, lights, blue, sunlight);
}

// set spotlight data; position and direction are mandatory, other values have defaults
// the view*projection matrix is updated uatomatically from the position and direction
// position and direction are in world coordinates
void LightsAndShadows::setSpotlight( int which, DirectX::FXMVECTOR position, DirectX::FXMVECTOR direction, float beamHalfAngle /*= (float)M_PI * 15.0f/180.f*/, float constantAttenuation /*= 0.75f*/, float linearAttenuation /*= 0.0f*/, float quadraticAttenuation /*= 0.05f*/ )
{
    assert(which < NUM_SPOTLIGHTS); // for directional lights use pointMoonlight()

    lights[which].move(position, direction);
    lights[which].setHalfAngle(beamHalfAngle);
}

// this method sets the direction of the directional light
// it also updates the relevant shadow information and therefore needs player camera data in order to optimize shadowmap placement
void LightsAndShadows::pointMoonlight( DirectX::CXMMATRIX direction, FirstPerson &FPCamera )
{
    // wide view directional shadow
    XMVECTOR skyCamPos = FPCamera.getEyePosition();
    skyCamPos = XMVectorFloor(skyCamPos + XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f) - lightDirection * 90);

    XMMATRIX directionalShadowOrtho = XMMatrixLookToLH(skyCamPos, 
        lightDirection, XMVectorSet(0, 0, 1, 0)) * XMMatrixOrthographicLH(100, 100, 0.1f, 1000); // view * orthographic projection, for directional light
    // N.B., "up" should be orthogonal to lightDirection, or at least not parallel because a crossproduct is performed

    XMStoreFloat4x4(&lights[NUM_SPOTLIGHTS].projection, directionalShadowOrtho);

    // high LOD view for directional shadow, focused on the player's vicinity
    skyCamPos = FPCamera.getEyePosition();
    skyCamPos = XMVectorFloor(skyCamPos  + XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f) - lightDirection * 90 + FPCamera.getForwardVector()*3.5);
    directionalShadowOrtho = XMMatrixLookToLH(skyCamPos, 
        lightDirection, XMVectorSet(0, 0, 1, 0)) * XMMatrixOrthographicLH(7, 7, 0.1f, 1000); // view * orthographic projection, for directional light

    XMStoreFloat4x4(&lights[NUM_SPOTLIGHTS+1].projection, directionalShadowOrtho);
}


// XXX refactor in the future:
bool doRenderCalls( ModelManager & models, D3DClass &d3d, VanillaShaderClass & shader, CXMMATRIX view, CXMMATRIX projection, std::vector<Light> &lights);

bool LightsAndShadows::renderShadowMaps( D3DClass &d3d, ModelManager & models )
{
    //
    // make shadow maps
    //

    ID3D11ShaderResourceView *nil[NUM_SPOTLIGHTS+2];
    ZeroMemory(nil, sizeof(nil));
    d3d.GetDeviceContext()->PSSetShaderResources(3, NUM_SPOTLIGHTS+2, nil); // unbind shadow buffers from input

    // Set up the viewport for rendering at shadow map resolution
    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float)ShadowBuffer::width, (float)ShadowBuffer::height, 0.0f, 1.0f };
    d3d.GetDeviceContext()->RSSetViewports(1, &viewport);
    d3d.setDepthBias(true);

    // re-render scene once for every shadow
    int i;
    auto light = lights.begin();
    auto shadow = shadows.begin();
    for (; light != lights.end() && shadow != shadows.end(); ++light, ++shadow)
    {
        XMMATRIX view = XMLoadFloat4x4(&lights[i].projection);

        shadows[i].setAsRenderTarget(d3d.GetDeviceContext());
        shadows[i].clear(d3d.GetDeviceContext());

        if (!doRenderCalls(models, d3d, shadowShaders, view, XMMatrixIdentity(), lights)) return false;
    }

    //
    // The directional light shadowmaps may be lighting a very large outdoor area, so they may require higher resolutions.
    //
    D3D11_VIEWPORT viewport2 = { 0.0f, 0.0f, (float)shadows[NUM_SPOTLIGHTS].m_width, (float)shadows[NUM_SPOTLIGHTS].m_height, 0.0f, 1.0f };
    d3d.GetDeviceContext()->RSSetViewports(1, &viewport2);
    XMMATRIX view = XMLoadFloat4x4(&(lights[NUM_SPOTLIGHTS+1].projection));

    shadows[NUM_SPOTLIGHTS].setAsRenderTarget(d3d.GetDeviceContext());
    shadows[NUM_SPOTLIGHTS].clear(d3d.GetDeviceContext());
    if (!doRenderCalls(models, d3d, shadowShaders, view, XMMatrixIdentity(), lights)) return false;

    D3D11_VIEWPORT viewport3 = { 0.0f, 0.0f, (float)shadows[NUM_SPOTLIGHTS+1].m_width, (float)shadows[NUM_SPOTLIGHTS+1].m_height, 0.0f, 1.0f };
    d3d.GetDeviceContext()->RSSetViewports(1, &viewport3);
    view = XMLoadFloat4x4(&(lights[NUM_SPOTLIGHTS+1].projection));

    shadows[NUM_SPOTLIGHTS+1].setAsRenderTarget(d3d.GetDeviceContext());
    shadows[NUM_SPOTLIGHTS+1].clear(d3d.GetDeviceContext());
    if (!doRenderCalls(models, d3d, shadowShaders, view, XMMatrixIdentity(), lights)) return false;
}

void LightsAndShadows::setShadowsAsViewResources( D3DClass &d3d )
{
    int numShadows;
    for (numShadows = 0; lights[numShadows].enabled; ++numShadows); // this seems dumb XXX
    ShadowBuffer::pushToGPU(d3d.GetDeviceContext(), shadows, numShadows); // the DepthStencilView must be re-bound before this call! e.g., the offScreen.setAsRenderTarget() call does it
}
