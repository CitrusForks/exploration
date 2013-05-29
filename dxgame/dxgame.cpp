// dxgame.cpp : DirectX testing ground, entry point
//

#include "stdafx.h"

// define this to debug main shaders with VS2012:
// #define DISABLE_OFFSCREEN_BUFFER 1
// 


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <dxgi.h>
#include <d3d11.h>

#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

#include <lua.hpp>
#include <FW1FontWrapper.h>
#include <assimp/Importer.hpp>

#include "d3dclass.h"
#include "vanillashaderclass.h"
#include "LoadedTexture.h"
#include "SimpleMesh.h"
//#include "ComplexMesh.h"
#include "CompoundMesh.h"
#include "Sound.h"
#include "Input.h"
#include "FirstPerson.h"
#include "SimpleText.h"
#include "ShadowBuffer.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// extra dependencies:
#pragma comment(lib, "lua52.lib")
#pragma comment(lib, "FW1FontWrapper.lib")
#pragma comment(lib, "assimp.lib")


using namespace std;
using namespace DirectX;

ostream& operator << ( ostream& os, CXMMATRIX m) 
{ 
    XMFLOAT4X4 mm;
    XMStoreFloat4x4(&mm, m);

    for( int i = 0; i < 4; ++ i) 
    { 
        for( int j = 0; j < 4; ++ j) os << mm.m[i][j] << "\t"; 
        os << endl; 
    } 
    return os; 
}

ostream& operator << ( ostream& os, FXMVECTOR v)
{
    XMFLOAT4 vv;
    XMStoreFloat4(&vv, v);
    return os << vv.x << "\t" << vv.y << "\t" << vv.z << "\t" << vv.w << endl;
}



LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch(umessage)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_CLOSE:
			PostQuitMessage(0);		
			break;

		case WM_CREATE:

		default:
			return DefWindowProc(hwnd, umessage, wparam, lparam);

			// TODO: pass messages to real handler in program
			break;
	}

	return 0;
}


void reportError(const char *prefix)
{
	int error = GetLastError();
	char errorStr[256];
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, 0, errorStr, 255, nullptr);
	cout << prefix << errorStr << endl;
}

bool RenderScene( D3DClass &d3d, FirstPerson &FPCamera, FXMVECTOR axis45deg, VanillaShaderClass &shaders0, XMFLOAT3 lightDirection, Chronometer &timer, IntermediateRenderTarget &offScreen, ModelManager &models, CXMMATRIX projection, SimpleMesh &square, VanillaShaderClass &postProcess, CXMMATRIX ortho, SimpleText &text, VanillaShaderClass &shadowShaders, ShadowBuffer *shadows, int shadowsNum, CXMMATRIX directionalShadowOrtho );


int _tmain(int argc, _TCHAR* argv[])
{
    Options::setDefaults();
	int width = Options::intOptions["Width"], height = Options::intOptions["Height"];
#ifdef DISABLE_OFFSCREEN_BUFFER
	Options::intOptions["MSAACount"] = 1;
	Options::intOptions["MSAAQuality"] = 0;
#endif

	wcout << L"Unicode test: проверка Unicode" << endl; // unlikely to work! need to manually set codepage in terminal
	cout << endl; // because the above is unlikely to work

	HMODULE progInstance = GetModuleHandle(nullptr);
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = progInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"MainClass";
	wc.cbSize = sizeof(WNDCLASSEX);

	ATOM wcAtom = RegisterClassEx(&wc); // register for class, do not waitlist. 
	if (!wcAtom)
	{
		cout << "Error in RegisterClassEx()" << endl;
	}

	HWND window = CreateWindowEx(WS_EX_APPWINDOW, L"MainClass", L"TODO: come up with title", WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP, 25, 25, width, height, HWND_DESKTOP, 0, progInstance, 0);
	if (!window)
	{
		reportError("Error in CreateWindowEx() ");
	}

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(window, SW_SHOW);
	SetForegroundWindow(window);
	SetFocus(window);


        // bring up actual game-related objects:

        Chronometer timer;

	D3DClass d3d;

        // D3DClass wraps some of the more tedious aspects of DirectX, we need that:
	d3d.Initialize(width, height, true, window, false, 32, 1.0f);

        // this is the off-screen buffer for drawing
        IntermediateRenderTarget offScreen(d3d.GetDevice(), d3d.GetDeviceContext(), width, height);
        if (!offScreen.getResourceViewAndResolveMSAA(d3d.GetDeviceContext()))
        {
            return 1;
        }

        // initialize the text renderer with our favorite stupid-looking font
        SimpleText text(d3d.GetDevice(), L"Special Elite");

        // Sound wraps FMOD for now and needs some work
        Sound soundSystem;

        // DirectInput wrapper
        Input input; input.Initialize(progInstance, window, width, height);

        // FirstPerson handles input for an FPS view and returns a view matrix or other related data
        FirstPerson FPCamera;
        //FPCamera.setPosition(XMVectorSet(0,0,-5,1));

        // Assimp loads almost any 3D format though it's a bit slow sometimes; still useful:
        Assimp::Importer modelImporter; // this object will own the memory allocated for the models it loads; when it's destroyed, memory is automatically deallocated

        // shaders0 holds the shaders for the main scene rendering step
	VanillaShaderClass shaders0;
	if (!shaders0.InitializeShader(d3d.GetDevice(), window, L"light_vs.cso",  L"light_ps.cso"))
        {
            return 1;
        }

        // shadowShaders contains a cut-down pixel shader to facilitate generating a shadow map from a depth buffer
        VanillaShaderClass shadowShaders;
        shadowShaders.InitializeShader(d3d.GetDevice(), window, L"light_vs.cso", L"shadow_ps.cso");

        // shadow maps! 
        vector<ShadowBuffer> shadowBuffers;
        shadowBuffers.resize(NUM_SPOTLIGHTS + 1); // +1 for directional light
        for (unsigned i = 0; i < shadowBuffers.size(); ++i)
        {
            shadowBuffers[i].init(d3d.GetDevice(), d3d.GetDeviceContext());
        }

        // this is the effects shader for rendering from off-screen buffer to swap chain
        VanillaShaderClass postProcess;
        if (!postProcess.InitializeShader(d3d.GetDevice(), window, L"postprocess_vs.cso", L"postprocess_ps.cso"))
        {
            return 1;
        }

        // we're lazy about loading files for now, just get everything from the data directory:
        SetCurrentDirectoryA(".\\data");

        // this is how sounds are loaded:
        int beepverb = soundSystem.loadSound("beepverb.wav");

        // models will hold all the models we load:
        ModelManager models(d3d.GetDevice(), d3d.GetDeviceContext());

        CompoundMesh *mesh = models["Chekov.obj"];

        CompoundMesh *duck = models["duck.obj"];

        CompoundMesh *floor = models["floor.obj"];

        CompoundMesh *torus = models["torus.obj"];

        CompoundMesh *building = models["LPBuildX13r_3ds.3ds"];

        if (!mesh || !duck || !floor || !torus || !building) return -1;

        // WARNING, these pointers are invalid since the vector holding them has probably been reallocated

        SimpleMesh square;
        if (!square.load(L"square.obj", d3d.GetDevice()))
        {
            return 1;
        }

        XMMATRIX projection = XMMatrixPerspectiveFovLH((float)((60.0/360.0) * M_PI * 2), (float)width / (float)height, 0.1f, 1000.0f);

        // cout << XMMatrixTranspose(projection);

        //cout << XMVector3Transform(XMVectorSet(0,0,2,1), projection);


        XMMATRIX ortho = XMMatrixOrthographicOffCenterLH(0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 1.1f);

        XMFLOAT3 lightDirection(0.0f,  -1.0f, 0.0f); // directional light

        XMMATRIX directionalShadowOrtho = XMMatrixLookToLH(FPCamera.getEyePosition() + XMVectorSet(0, 100.0f, 0, 0), 
            XMLoadFloat3(&lightDirection), XMVectorSet(0, 1, 0, 0)) * XMMatrixOrthographicLH(50, 50, 0.1, 1000); // view * orthographic projection, for directional light

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG)); // clear message structure
	
        XMVECTOR axis45deg = XMVectorSet(-0.7071067811865475f, -0.7071067811865475f, 0.0f, 0.0f); // normalized 45 degree angle vector

        float angle = 0.0f;
	bool done = false;
	while (!done)
	{
                timer.Sample(); // read timer and update variables

                input.Frame(); // read input and update state

                FPCamera.perFrameUpdate(timer.sincePrev(), input); // move the camera

                soundSystem.perFrameUpdate();

		// Handle the windows messages.
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if(msg.message == WM_QUIT)
		{
			done = true;
		}

                if (!RenderScene(d3d, FPCamera, axis45deg, shaders0, lightDirection, timer, offScreen, models, projection, square, postProcess, ortho, text, shadowShaders, shadowBuffers.data(), shadowBuffers.size(), directionalShadowOrtho)) return -1;

		//Sleep(10); // don't cook the CPU yet
                angle += (float)(M_PI) * (float)timer.sincePrev();


                static double last_honked = 0.0;
                if (input.IsPressed(DIK_E) && timer.sinceInit() - last_honked > 0.5)
                {
                    soundSystem.play(beepverb);
                    last_honked = timer.sinceInit();
                }


                if (input.IsPressed(DIK_ESCAPE)) done = true;

                if (input.IsPressed(DIK_LALT) && input.IsPressed(DIK_F4)) done = true; // be nice
	}

        // need some kind of container to take care of all this cleanup...
        // perhaps a garbage bag
	shaders0.Shutdown();
        postProcess.Shutdown();
        shadowShaders.Shutdown();

        for (int i = 0; i < NUM_SPOTLIGHTS + 1; ++i)
        {
            shadowBuffers[i].release();
        }

        offScreen.Shutdown();

        text.Release();
        text.ReleaseFactory();

	d3d.Shutdown();

	DestroyWindow(window);
	UnregisterClass(L"MainClass", progInstance);

	return 0;
}

bool doRenderCalls( ModelManager & models, D3DClass &d3d, VanillaShaderClass & shader, CXMMATRIX view, CXMMATRIX projection, CXMMATRIX worldFinal, XMFLOAT4X4 *lightProjections, unsigned numLights );

bool RenderScene( D3DClass &d3d, FirstPerson &FPCamera, FXMVECTOR axis45deg, VanillaShaderClass &shaders0, XMFLOAT3 lightDirection, Chronometer &timer, IntermediateRenderTarget &offScreen, ModelManager &models, CXMMATRIX projection, SimpleMesh &square, VanillaShaderClass &postProcess, CXMMATRIX ortho, SimpleText &text, VanillaShaderClass &shadowShaders, ShadowBuffer *shadows, int shadowsNum, CXMMATRIX directionalShadowOrtho )
{
    //
    // Rendering
    // 

#ifndef DISABLE_OFFSCREEN_BUFFER
    d3d.BeginScene(false); // don't clear back buffer; we're just going to overwrite it completely from the off-screen buffer
#else
	d3d.BeginScene(true);
#endif

    //
    // Spotlights! 
    // TODO: move them
    // 

    XMFLOAT4 lightPos[NUM_SPOTLIGHTS+1]; // positions
    ZeroMemory(lightPos, sizeof(lightPos));
    XMStoreFloat4(lightPos, XMVector4Transform(FPCamera.getEyePosition(), XMMatrixTranslation(0.1f, -0.3, 0.1))); // hold flashlight lower
    lightPos->y += 0.2f;
    lightPos[1] = XMFLOAT4(4.0f, 4.0f, 3.0f, 1.0f);

    XMFLOAT3 lightDir[NUM_SPOTLIGHTS+1]; // directions
    ZeroMemory(lightDir, sizeof(lightDir));
    XMStoreFloat3(lightDir, XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMMatrixRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), -2.0f/180 * (float)M_PI) *  XMMatrixTranspose(FPCamera.getViewMatrix())));
    XMStoreFloat3(&(lightDir[1]), axis45deg);

    float lightHalfAngles[NUM_SPOTLIGHTS+1] = { (float)M_PI * 15.0f/180.f, (float)M_PI * 25.0f/180.f, 0.0f, 0.0f }; // angles from axis to generatrix

    XMFLOAT4 lightParams[NUM_SPOTLIGHTS+1]; // { cos(halfAngle), constant attenuation, linear att., quadratic att. }
    ZeroMemory(lightParams, sizeof(lightParams));
    lightParams[0] = XMFLOAT4(cosf(lightHalfAngles[0]), 0.75f, 0, 0.05f); // player flashlight
    lightParams[1] = XMFLOAT4(cosf(lightHalfAngles[1]), 0.75f, 0.1f, 0);  // fixed floating light

    int numLights = 2;

    XMFLOAT4X4 lightProj[NUM_SPOTLIGHTS+1]; // View*Projection matrices ("VP") for all the lights, >>>including the directional light in the final position<<<

    for (int i = 0; i < numLights; ++i)
    {
        XMMATRIX viewProj = XMMatrixLookToLH(XMLoadFloat4(lightPos+i), XMLoadFloat3(lightDir+i), XMVectorSet(0, 1, 0, 0)) * 
            XMMatrixPerspectiveFovLH(lightHalfAngles[i]*2, 1.0f /* shadowmap is a square at any res. */, 0.1f, 1000.0f);
        XMStoreFloat4x4(lightProj+i, viewProj);
    }

    XMStoreFloat4x4(lightProj+NUM_SPOTLIGHTS, directionalShadowOrtho);

    shaders0.SetPSLights(d3d.GetDeviceContext(), lightDirection, (float)timer.sinceInit(), FPCamera.getEyePosition(), lightPos, lightDir, lightParams, 2);

    XMMATRIX world = XMMatrixTranslation(0.0f, 0.0f, 7.0f);

    XMMATRIX worldFinal = /* XMMatrixRotationAxis(axis, angle) * */ world;

    //
    // make shadow maps
    //

    ID3D11ShaderResourceView *nil[NUM_SPOTLIGHTS+1];
    ZeroMemory(nil, sizeof(nil));
    d3d.GetDeviceContext()->PSSetShaderResources(3, NUM_SPOTLIGHTS+1, nil); // unbind shadow buffers from input

    // Set up the viewport for rendering at shadow map resolution
    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float)ShadowBuffer::width, (float)ShadowBuffer::height, 0.0f, 1.0f };
    d3d.GetDeviceContext()->RSSetViewports(1, &viewport);
    d3d.setDepthBias(true);

    // re-render scene once for every shadow
    int i;
    for (i = 0; i < shadowsNum && i < numLights; ++i)
    {
        XMMATRIX view = XMLoadFloat4x4(&(lightProj[i]));

        shadows[i].setAsRenderTarget(d3d.GetDeviceContext());
        shadows[i].clear(d3d.GetDeviceContext());

        if (!doRenderCalls(models, d3d, shadowShaders, view, XMMatrixIdentity(), worldFinal, lightProj, numLights)) return false;
    }

    XMMATRIX view = XMLoadFloat4x4(&(lightProj[NUM_SPOTLIGHTS]));
    shadows[NUM_SPOTLIGHTS].setAsRenderTarget(d3d.GetDeviceContext());
    shadows[NUM_SPOTLIGHTS].clear(d3d.GetDeviceContext());
    if (!doRenderCalls(models, d3d, shadowShaders, view, XMMatrixIdentity(), worldFinal, lightProj, numLights)) return false;


    //
    // render the scene to off-screen buffer
    // 
    D3D11_VIEWPORT viewport2 = { 0.0f, 0.0f,  (float)Options::intOptions["Width"], (float)Options::intOptions["Height"], 0.0f, 1.0f};
    d3d.GetDeviceContext()->RSSetViewports(1, &viewport2);

    view = FPCamera.getViewMatrix();

    shaders0.setVSCameraBuffer(d3d.GetDeviceContext(), FPCamera.getEyePosition(), (float)timer.sinceInit(), 0);


#ifndef DISABLE_OFFSCREEN_BUFFER
    offScreen.setAsRenderTarget(d3d.GetDeviceContext(), d3d.GetDepthStencilView()); // set the off-screen texture as the render target
    offScreen.clear(d3d.GetDeviceContext());
#else
	d3d.setAsRenderTarget(true);
	d3d.depthOn();
#endif

    ShadowBuffer::pushToGPU(d3d.GetDeviceContext(), shadows, shadowsNum); // the DepthStencilView must be re-bound before this call! e.g., the offScreen.setAsRenderTarget() call does it
    d3d.setDepthBias(false);


    if (!doRenderCalls(models, d3d, shaders0, view, projection, worldFinal, lightProj, numLights)) return false;
    //if (!doRenderCalls(models, d3d, shaders0, XMMatrixLookToLH(FPCamera.getEyePosition() + XMVectorSet(0, 100.0f, 0, 0), 
    //    XMLoadFloat3(&lightDirection), XMVectorSet(0, 1, 0, 0)), XMMatrixOrthographicLH(50, 50, 0.1, 1000), worldFinal, lightProj, numLights)) return false;
    
    //
    // Done rendering scene
    //

#ifndef DISABLE_OFFSCREEN_BUFFER
    d3d.depthOff(); // disable depth test

    d3d.setAsRenderTarget(false); // set a swap chain buffer as render target again

    square.setBuffers(d3d.GetDeviceContext()); // use the two triangles to render off-screen texture to swap chain, through a shader
    if (!postProcess.Render(d3d.GetDeviceContext(), square.getIndexCount(), XMMatrixIdentity(), XMMatrixIdentity(), ortho, nullptr, nullptr, lightProj, numLights, offScreen.getResourceViewAndResolveMSAA(d3d.GetDeviceContext()), 1, false))
    //if (!postProcess.Render(d3d.GetDeviceContext(), square.getIndexCount(), XMMatrixIdentity(), XMMatrixIdentity(), ortho, nullptr, nullptr, lightProj, numLights, shadows[NUM_SPOTLIGHTS].getResourceView(d3d.GetDeviceContext()), 1, false)) // comment out previous line and uncomment this one to visually inspect a shadow map
    {
        Errors::Cry(L"Error rendering off-screen texture to display. :/");
        return false;
    }
#endif

    text.write(d3d.GetDeviceContext(), L"Hello?", 0, 0);

    d3d.depthOn(); // enable depth test again for normal drawing

    d3d.EndScene();

    return true;
}


// some kind of function that dispatches the render calls to models in the model manager
// it needs to be called once per shadow map and then once more for the final scene
bool doRenderCalls( ModelManager & models, D3DClass &d3d, VanillaShaderClass & shader, CXMMATRIX view, CXMMATRIX projection, CXMMATRIX worldFinal, XMFLOAT4X4 *lightProjections, unsigned numLights )
{
    if (!models["Chekov.obj"]->render(d3d.GetDeviceContext(), &shader, XMMatrixScaling(0.53f, 0.53f, 0.53f) * XMMatrixTranslation(0.0f, 0.0f, 7.0f),  view, projection, lightProjections, numLights))
    {
        Errors::Cry(L"Render error in scene. :|");
        return false;
    }

    //if (!spider.Render(d3d.GetDeviceContext(), &shaders0, FPCamera.getPosition(), XMMatrixScaling(0.05f, 0.05f, 0.05f) * worldFinal * XMMatrixTranslation(-1.0f, 2.0f, 6.0f), view, projection))
    if (!models["duck.obj"]->render(d3d.GetDeviceContext(), &shader, worldFinal * XMMatrixTranslation(-1.0f, 0.0f, 3.0f), view, projection, lightProjections, numLights))
    {
        Errors::Cry(L"Render error in scene. :|");
        return false;
    }
#if 1
    if (!models["floor.obj"]->render(d3d.GetDeviceContext(), &shader, XMMatrixIdentity(), view, projection, lightProjections, numLights))
    {
        Errors::Cry(L"Render error in scene. :|");
        return false;
    }
#endif
    //shaders0.setVSCameraBuffer(d3d.GetDeviceContext(), FPCamera.getEyePosition(), timer.sinceInit(), 1);

    if (!models["torus.obj"]->render(d3d.GetDeviceContext(), &shader, XMMatrixTranslation(0.0f, 1.0f, 3.0f), view, projection, lightProjections, numLights))
    {
        Errors::Cry(L"Render error in scene. :|");
        return false;
    }

    //shaders0.setVSCameraBuffer(d3d.GetDeviceContext(), FPCamera.getEyePosition(), timer.sinceInit(), 0);
    if (!models["LPBuildX13r_3ds.3ds"]->render(d3d.GetDeviceContext(), &shader, XMMatrixRotationAxis(XMVectorSet(1.0f,0,0,0), (float)M_PI_2) * XMMatrixScaling(0.15f, 0.15f, 0.15f) * XMMatrixTranslation(-10.0f, 4.5001f, 15.0f), view, projection, lightProjections, numLights))
    {
        Errors::Cry(L"Render error in scene. :|");
        return false;
    }

    return true;
}

