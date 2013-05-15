// dxgame.cpp : DirectX testing ground, entry point
//

#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <dxgi.h>
#include <d3d11.h>

#include <iostream>

#include "d3dclass.h"
#include "textureshaderclass.h"
#include "textureclass.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "dxgi.lib")


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

	std::wcout << L"Unicode test: проверка Unicode" << std::endl;
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

	D3DClass d3d;

	d3d.Initialize(width, height, true, window, false, 32, 1.0f);

	TextureShaderClass shaders0;
	shaders0.InitializeShader(d3d.GetDevice(), window, L"ColorVertexShader.vs", "ColorVertexShader",  L"ColorPixelShader.ps", "ColorPixelShader");

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG)); // clear message structure
	
	bool done = false;
	while (!done)
	{
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

		Sleep(10); // don't cook the CPU yet
	}

	shaders0.Shutdown();

	d3d.Shutdown();

	DestroyWindow(window);
	UnregisterClass(L"MainClass", progInstance);

	return 0;
}

