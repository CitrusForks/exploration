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

#include "d3dclass.h"
#include "vanillashaderclass.h"
#include "textureclass.h"
#include "SimpleMesh.h"
#include "Sound.h"
#include "inputclass.h"
#include "FirstPerson.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "lua52.lib")


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
	std::cout << prefix << errorStr << std::endl;
}




int _tmain(int argc, _TCHAR* argv[])
{
	int width = 1024, height = 768;

	std::wcout << L"Unicode test: проверка Unicode" << std::endl; // unlikely to work! need to manually set codepage in terminal
	std::cout << std::endl; // because the above is unlikely to work

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
		std::cout << "Error in RegisterClassEx()" << std::endl;
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

        Sound soundSystem;

        InputClass input; input.Initialize(progInstance, window, width, height);

        FirstPerson FPCamera;

        int beepverb = soundSystem.loadSound("data/beepverb.wav");

	VanillaShaderClass shaders0;
	if (!shaders0.InitializeShader(d3d.GetDevice(), window, L"light.vs", "LightVertexShader",  L"light.ps", "LightPixelShader"))
        {
            return 1;
        }

        SimpleMesh mesh;
        if (!mesh.load(L"duck.obj", d3d.GetDevice()))
        {
            return 1;
        }

        TextureClass texture;
        texture.Initialize(d3d.GetDevice(), d3d.GetDeviceContext(), L"duck_texture.png");
        
        XMMATRIX world = XMMatrixTranslation(0.0f, 0.0f, 7.0f);

        XMMATRIX projection = XMMatrixPerspectiveFovLH((float)((60.0/360.0) * M_PI * 2), (float)width / (float)height, 1.0f, 1000.0f);

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG)); // clear message structure
	
        XMVECTOR axis = XMVectorSet(0.7071067811865475f, 0.7071067811865475f, 0.0f, 0.0f); // normalized 45 degree angle vector

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

                //d3d.BeginScene(0.0f, sinf(angle), sinf(angle+3.141592653589f), 0.0f);
                d3d.BeginScene(0.0f, 0.0f, 0.0f, 0.0f);
                mesh.setBuffers(d3d.GetDeviceContext());
                
                shaders0.SetPSConstants(d3d.GetDeviceContext(), XMFLOAT4(0.25f, 0.2f, 0.2f, 1.0f), XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), 100.0f, XMFLOAT4(0.0f, 0.0f, 0.75f, 1.0f), (float)timer.sinceInit());


                XMMATRIX worldFinal = /* XMMatrixRotationAxis(axis, angle) * */ world;

                XMMATRIX view = FPCamera.getViewMatrix();

                if (!shaders0.Render(d3d.GetDeviceContext(), mesh.getIndexCount(), worldFinal, view, projection, texture.GetTexture(), FPCamera.getPosition()))
                {
                    std::cout << "Render error!";
                    break;
                }
                d3d.EndScene();


		Sleep(4); // don't cook the CPU yet
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

	shaders0.Shutdown();

	d3d.Shutdown();

	DestroyWindow(window);
	UnregisterClass(L"MainClass", progInstance);

	return 0;
}

