#include "stdafx.h"
#include "Graphics.h"
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;


Graphics::Graphics(int width, int height, HWND window, HMODULE progInstance) : FPCamera()
{
    // D3DClass wraps some of the more tedious aspects of DirectX, we need that:
    d3d.Initialize(width, height, true, window, false, 32, 1.0f);

    // initialize the text renderer with our favorite stupid-looking font
    text.init(d3d.GetDevice(), L"Special Elite");

    // this is the off-screen buffer for drawing
    offScreen = IntermediateRenderTarget(d3d.GetDevice(), d3d.GetDeviceContext(), width, height);
    if (!offScreen.getResourceViewAndResolveMSAA(d3d.GetDeviceContext()))
    {
        Errors::Cry("Couldn't initialize off-screen buffer; something muts be really wrong. Do you have a graphics card installed?");
    }

    shaders0.InitializeShader(d3d.GetDevice(), window, L"light_vs.cso",  L"light_ps.cso");

    postProcess.InitializeShader(d3d.GetDevice(), window, L"postprocess_vs.cso", L"postprocess_ps.cso");

    projection = XMMatrixPerspectiveFovLH((float)((60.0/360.0) * M_PI * 2), (float)width / (float)height, 0.1f, 1000.0f);

    ortho = XMMatrixOrthographicOffCenterLH(0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 1.1f);

    if (!square.load(L"square.obj", d3d.GetDevice()))
    {
        return;
    }
}

Graphics::Graphics( Graphics & )
{
    Errors::Cry("You may not copy the Graphics object.");
}

Graphics & Graphics::operator=( Graphics & )
{
    Errors::Cry("You may not assign the Graphics object with operator= ! Try using a shared_ptr<>");
    return *this;
}



Graphics::~Graphics(void)
{
    // need some kind of container to take care of all this cleanup...
    // perhaps a garbage bag
    shaders0.Shutdown();
    postProcess.Shutdown();

    offScreen.Shutdown();

    text.Release();
    text.ReleaseFactory();

    d3d.Shutdown();
}

bool Graphics::RenderScene( Chronometer &timer, Scene *scene)
{
    shared_ptr<LightsAndShadows> lighting = scene->getLights();

    //
    // Rendering
    // 


#ifndef DISABLE_OFFSCREEN_BUFFER
    d3d.BeginScene(false); // don't clear back buffer; we're just going to overwrite it completely from the off-screen buffer
#else
    d3d.BeginScene(true);
#endif

#if 0
    sunlight.x *=2;
    sunlight.y *=2;
    sunlight.z *=2;
#endif


    lighting->updateGPU(d3d.GetDeviceContext(), shaders0, (float)timer.sinceInit(), FPCamera.getEyePosition());

    if (!lighting->renderShadowMaps(d3d, [&](VanillaShaderClass &shader, CXMMATRIX view, CXMMATRIX projection, std::vector<Light> &lights)
    {
        Scene::renderFunc_t renderFuncForScene = [&] (CXMMATRIX world, shared_ptr<ModelManager> models, int modelRefNum, shared_ptr<LightsAndShadows> lighting) 
        {
            return (*models)[modelRefNum]->render(d3d.GetDeviceContext(), &shaders0, world,  view, projection, lights);
        };

        //return doRenderCalls(scene, d3d, shader, view, projection, lights); 
        return scene->render(renderFuncForScene);
    }
    )) return false;


    //
    // render the scene to off-screen buffer
    // 
    D3D11_VIEWPORT viewportMain = { 0.0f, 0.0f,  (float)Options::intOptions["Width"], (float)Options::intOptions["Height"], 0.0f, 1.0f};
    d3d.GetDeviceContext()->RSSetViewports(1, &viewportMain);

    XMMATRIX view = FPCamera.getViewMatrix();

    shaders0.setVSCameraBuffer(d3d.GetDeviceContext(), FPCamera.getEyePosition(), (float)timer.sinceInit(), 0);


#ifndef DISABLE_OFFSCREEN_BUFFER
    offScreen.setAsRenderTarget(d3d.GetDeviceContext(), d3d.GetDepthStencilView()); // set the off-screen texture as the render target
    offScreen.clear(d3d.GetDeviceContext());
#else
    d3d.setAsRenderTarget(true);
    d3d.depthOn();
#endif

    lighting->setShadowsAsViewResources(d3d);
    d3d.setDepthBias(false);

    Scene::renderFunc_t renderFuncForScene = [&] (CXMMATRIX world, shared_ptr<ModelManager> models, int modelRefNum, shared_ptr<LightsAndShadows> lighting) 
    {
        return (*models)[modelRefNum]->render(d3d.GetDeviceContext(), &shaders0, world,  view, projection, lighting->getLights());
    };

    if (!scene->render(renderFuncForScene)) return false; // XXX style: use exceptions instead?

    //
    // Done rendering scene
    //

#ifndef DISABLE_OFFSCREEN_BUFFER

    //
    // Copy off-screen buffer to swap chain buffer

    d3d.depthOff(); // disable depth test

    d3d.setAsRenderTarget(false); // set a swap chain buffer as render target again

    square.setBuffers(d3d.GetDeviceContext()); // use the two triangles to render off-screen texture to swap chain, through a shader
    if (!postProcess.Render(d3d.GetDeviceContext(), square.getIndexCount(), XMMatrixIdentity(), XMMatrixIdentity(), ortho, nullptr, nullptr, nullptr, offScreen.getResourceViewAndResolveMSAA(d3d.GetDeviceContext()), 1, false))
        //if (!postProcess.Render(d3d.GetDeviceContext(), square.getIndexCount(), XMMatrixIdentity(), XMMatrixIdentity(), ortho, nullptr, nullptr, nullptr, lighting.getShadows()[NUM_SPOTLIGHTS].getResourceView(d3d.GetDeviceContext()), 1, false)) // comment out previous line and uncomment this one to visually inspect a shadow map
    {
        Errors::Cry(L"Error rendering off-screen texture to display. :/");
        return false;
    }
#endif

    //text.write(d3d.GetDeviceContext(), L"Hello?", 0, 0);
    wchar_t fps[256];
    StringCbPrintfW(fps, sizeof(fps), L"%.02f fps", 1.0f / (float)timer.sincePrev());
    //text.write(d3d.GetDeviceContext(), fps, 25, 25);

    d3d.depthOn(); // enable depth test again for normal drawing

    d3d.EndScene();

    return true;
}

