// dxgame.cpp : DirectX testing ground, entry point
//

#include "stdafx.h"

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

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// extra dependencies:
#pragma comment(lib, "lua52.lib")
#pragma comment(lib, "FW1FontWrapper.lib")
#pragma comment(lib, "assimp.lib")


using namespace std;
using namespace DirectX;


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


int _tmain(int argc, _TCHAR* argv[])
{
	int width = 1024, height = 768;

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

	d3d.Initialize(width, height, true, window, false, 32, 1.0f);

        IntermediateRenderTarget offScreen(d3d.GetDevice(), d3d.GetDeviceContext(), width, height);
        if (!offScreen.getResourceView(d3d.GetDeviceContext()))
        {
            return 1;
        }

        SimpleText text(d3d.GetDevice(), L"Special Elite");

        Sound soundSystem;

        Input input; input.Initialize(progInstance, window, width, height);

        FirstPerson FPCamera;
        //FPCamera.setPosition(XMVectorSet(0,0,-5,1));

        Assimp::Importer modelImporter; // this object will own the memory allocated for the models it loads; when it's destroyed, memory is automatically deallocated

	VanillaShaderClass shaders0;
	if (!shaders0.InitializeShader(d3d.GetDevice(), window, L"light.vs", "LightVertexShader",  L"light.ps", "LightPixelShader"))
        {
            return 1;
        }

        VanillaShaderClass postProcess;
        if (!postProcess.InitializeShader(d3d.GetDevice(), window, L"postprocess.vs", "PostProcVShader", L"postprocess.ps", "PostProcPixelShader"))
        {
            return 1;
        }

        SetCurrentDirectoryA(".\\data");

        int beepverb = soundSystem.loadSound("beepverb.wav");

        TextureManager textures;

        CompoundMesh mesh;
        if (!mesh.load(d3d.GetDevice(), d3d.GetDeviceContext(), &textures, "Chekov.obj"))
        {
            return 1;
        }

        CompoundMesh spider;
        if (!spider.load(d3d.GetDevice(), d3d.GetDeviceContext(), &textures, "duck.obj"))
        {
            return 1;
        }

        CompoundMesh floor;
        floor.load(d3d.GetDevice(), d3d.GetDeviceContext(), &textures, "floor.obj");

        CompoundMesh torus;
        torus.load(d3d.GetDevice(), d3d.GetDeviceContext(), &textures, "torus.obj");

        CompoundMesh building;
        building.load(d3d.GetDevice(), d3d.GetDeviceContext(), &textures, "LPBuildX13r_3ds.3ds");

        SimpleMesh square;
        if (!square.load(L"square.obj", d3d.GetDevice()))
        {
            return 1;
        }

        XMMATRIX world = XMMatrixTranslation(0.0f, 0.0f, 7.0f);

        XMMATRIX projection = XMMatrixPerspectiveFovLH((float)((60.0/360.0) * M_PI * 2), (float)width / (float)height, 0.1f, 1000.0f);

        XMMATRIX ortho = XMMatrixOrthographicOffCenterLH(0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 1.1f);

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG)); // clear message structure
	
        XMVECTOR axis45deg = XMVectorSet(-0.7071067811865475f, -0.7071067811865475f, 0.0f, 0.0f); // normalized 45 degree angle vector

        XMFLOAT3 lightDirection(0.0f,  -1.0f, 0.0f); // directional light

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

                //
                // Rendering
                // 

                d3d.BeginScene(false); // don't clear back buffer; we're just going to overwrite it completely from the off-screen buffer

                //
                //      SPOTLIGHTS! TODO: move them
                // 
                XMFLOAT4 lightPos[2]; 
                XMStoreFloat4(lightPos, FPCamera.getEyePosition());
                lightPos->y += 0.2f;
                lightPos[1] = XMFLOAT4(4.0f, 4.0f, 3.0f, 1.0f);
                
                XMFLOAT3 lightDir[2];
                XMStoreFloat3(lightDir, XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMMatrixRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), 10.0f/180 * (float)M_PI) * XMMatrixTranspose(FPCamera.getViewMatrix())));
                XMStoreFloat3(&(lightDir[1]), axis45deg);
                
                XMFLOAT4 lightParams[2];
                lightParams[0] = XMFLOAT4(cosf((float)M_PI * 20.0f/180.f), 0.75f, 0, 0.01f);
                lightParams[0] = XMFLOAT4(cosf((float)M_PI * 30.0f/180.f), 0.75f, 0.1f, 0);

                shaders0.SetPSLights(d3d.GetDeviceContext(), lightDirection, (float)timer.sinceInit(), FPCamera.getPosition(), lightPos, lightDir, nullptr, 2);

                XMMATRIX worldFinal = /* XMMatrixRotationAxis(axis, angle) * */ world;

                XMMATRIX view = FPCamera.getViewMatrix();

                offScreen.setAsRenderTarget(d3d.GetDeviceContext(), d3d.GetDepthStencilView()); // set the off-screen texture as the render target
                offScreen.clear(d3d.GetDeviceContext());

                if (!mesh.Render(d3d.GetDeviceContext(), &shaders0, FPCamera.getPosition(), XMMatrixScaling(0.53f, 0.53f, 0.53f) * worldFinal, view, projection))
                {
                    Errors::Cry(L"Render error in scene. :|");
                    break;
                }

                //if (!spider.Render(d3d.GetDeviceContext(), &shaders0, FPCamera.getPosition(), XMMatrixScaling(0.05f, 0.05f, 0.05f) * worldFinal * XMMatrixTranslation(-1.0f, 2.0f, 6.0f), view, projection))
                if (!spider.Render(d3d.GetDeviceContext(), &shaders0, FPCamera.getPosition(), worldFinal * XMMatrixTranslation(-1.0f, 0.0f, 3.0f), view, projection))
                {
                    Errors::Cry(L"Render error in scene. :|");
                    break;
                }
#if 1
                if (!floor.Render(d3d.GetDeviceContext(), &shaders0, FPCamera.getPosition(), XMMatrixIdentity(), view, projection))
                {
                    Errors::Cry(L"Render error in scene. :|");
                    break;
                }
#endif
                if (!torus.Render(d3d.GetDeviceContext(), &shaders0, FPCamera.getPosition(), XMMatrixTranslation(0.0f, 1.0f, 3.0f), view, projection))
                {
                    Errors::Cry(L"Render error in scene. :|");
                    break;
                }

                if (!building.Render(d3d.GetDeviceContext(), &shaders0, FPCamera.getPosition(), XMMatrixRotationAxis(XMVectorSet(1.0f,0,0,0), (float)M_PI_2) * XMMatrixScaling(0.15f, 0.15f, 0.15f) * XMMatrixTranslation(-10.0f, 4.5001f, 15.0f), view, projection))
                {
                    Errors::Cry(L"Render error in scene. :|");
                    break;
                }

                // done rendering scene

                d3d.setAsRenderTarget(); // set a swap chain buffer as render target again
                
                d3d.depthOff(); // disable depth test

                square.setBuffers(d3d.GetDeviceContext()); // use the two triangles to render off-screen texture to swap chain, through a shader
                if (!postProcess.Render(d3d.GetDeviceContext(), square.getIndexCount(), XMMatrixIdentity(), XMMatrixIdentity(), ortho * XMMatrixTranslation(-1.0f,1.0f,0.0f) * XMMatrixScaling(0.5f,0.5f,0.5f), XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), nullptr, offScreen.getResourceView(d3d.GetDeviceContext()), 1, false))
                {
                    Errors::Cry(L"Error rendering off-screen texture to display. :/");
                    break;
                }

                text.write(d3d.GetDeviceContext(), L"Hello?", 0, 0);

                d3d.depthOn(); // enable depth test again for normal drawing

                d3d.EndScene();


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
	shaders0.Shutdown();
        postProcess.Shutdown();

        offScreen.Shutdown();

        text.Release();
        text.ReleaseFactory();

	d3d.Shutdown();

	DestroyWindow(window);
	UnregisterClass(L"MainClass", progInstance);

	return 0;
}

