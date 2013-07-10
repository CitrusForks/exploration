// dxgame.cpp : DirectX testing ground, entry point
//

#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <strsafe.h>

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
#include "LightsAndShadows.h"
#include "Light.h"
#include "Graphics.h"
#include "ScriptedActor.h"

// is this a terrible way to specify libraries for linking? I kind of like it now.
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// extra dependencies:
#pragma comment(lib, "lua52.lib")
#pragma comment(lib, "FW1FontWrapper.lib")
#pragma comment(lib, "assimp.lib")

using namespace std;
using namespace DirectX;
using namespace Luna;


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


void stackDump (lua_State *L) {
    int i;
    int top = lua_gettop(L);
    for (i = 1; i <= top; i++) {  /* repeat for each level */
        printf("[%d] ", i);
        int t = lua_type(L, i);
        switch (t) {

        case LUA_TSTRING:  /* strings */
            printf("`%s'", lua_tostring(L, i));
            break;

        case LUA_TBOOLEAN:  /* booleans */
            printf(lua_toboolean(L, i) ? "true" : "false");
            break;

        case LUA_TNUMBER:  /* numbers */
            printf("%g", lua_tonumber(L, i));
            break;

        default:  /* other values */
            printf("%s", lua_typename(L, t));
            break;

        }
        printf("  ");  /* put a separator */
    }
    printf("\n");  /* end the listing */
}



float test = 0.0f; // just testing.

int _tmain(int argc, _TCHAR* argv[])
{
    Options::setDefaults();
    int width = Options::intOptions["Width"], height = Options::intOptions["Height"];
#ifdef DISABLE_OFFSCREEN_BUFFER
    Options::intOptions["MSAACount"] = 1;
    Options::intOptions["MSAAQuality"] = 0;
#endif

    //XMFLOAT4 vectortest(2, 2, 2, 1);
    //cout << XMVector4NormalizeEst(XMLoadFloat4(&vectortest)) << endl; exit(0); // verification that XMVector4Normalize() also divides w by length

    wcout << L"Unicode test: проверка Unicode" << endl; // unlikely to work! need to manually set codepage in terminal so have fun with that
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
	Errors::Cry("Error in RegisterClassEx()");
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


    // we're lazy about loading files for now, just get everything from the data directory:
    SetCurrentDirectoryA(".\\data");

    // bring up actual game-related objects:

    // DirectInput wrapper
    Input input; input.Initialize(progInstance, window, width, height);


    // Sound wraps FMOD for now and needs some work
    Sound soundSystem;

    // this is how sounds are loaded:
    int beepverb = soundSystem.loadSound("beepverb.wav");

    // Graphics contains most of the implementation details of the 3D stuff
    Graphics gEngine(width, height, window, progInstance);

    // though you're welcome do to stuff like this:
    //shared_ptr<TextureManager> tm = make_shared<TextureManager>();
    //shared_ptr<ModelManager> mm = make_shared<ModelManager>(gEngine.getD3D());
    // but Scene initializes its own managers now if you want it to

    //SceneDemo scene(gEngine.getD3D());
    //SceneDemo *scene = nullptr;

    Chronometer timer;

    // create the Lua state and register our Lua-exposed classes
    lua_State *L = luaL_newstate();
    luaL_openlibs(L); // NOTE: This is not sandboxed. All os and filesystem libraries are exposed. You wouldn't want to load user-created mods in this state.
    lua_pushlightuserdata(L, (void*)&(gEngine.getD3D()));
    lua_setglobal(L, "d3d"); // store d3d for objects initialized from Lua; this seems really clunky but it works :|
    LunaShare<ScriptedScene>::Register(L); // "Scene" object
    LunaShare<ScriptedActor>::Register(L); // "Actor" object
    LunaShare<ScriptedCamera>::Register(L); // "Camera" object

    shared_ptr<ScriptedScene> scene = nullptr;

#if 0
    if (luaL_dostring(L, "scene = Scene(); print(scene)"))
    {
        stackDump(L);
        Errors::Cry("Could not instantiate a Scene object from Lua");
    }

    luaL_dostring(L, "actor = Actor(); print(actor); actor:moveTo(1,2,3); actor = nil");


    scene->replaceManagers(mm, tm); // doing this rather than going through all the effort to store two new shared_ptr<> types in Lua state
    // TODO: perhaps what's needed is a generic templated method of storing a shared_ptr<> in Lua userdata; check boost-dependent library for this?
#endif

    MSG msg;
    ZeroMemory(&msg, sizeof(MSG)); // clear message structure
	
    cout << "Starting!!!" << endl;

    if (luaL_dofile(L, "Start.lua"))
    {
        // error! :(
        Errors::Cry((char*) lua_tostring(L, -1));
    }

    lua_getglobal(L, "scene");
    assert(lua_isuserdata(L, -1));
    shared_ptr<ScriptedScene>* udata = static_cast<shared_ptr<ScriptedScene>*>(lua_touserdata(L, -1));
    assert(udata);
    assert(*udata);
    scene = *udata;
    lua_pop(L, 1);

    gEngine.FPCamera = scene->FPCam; // use the scene's camera object; actually swapping the two variables here would have no effect, it would just use the gEngine instance.

    float angle = 0.0f;
    bool done = false;
    while (!done)
    {
        // Handle the windows messages.
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            // If windows signals to end the application then exit out.
            if(msg.message == WM_QUIT)
            {
                done = true;
            }

            continue; // ... all the messages! before we draw a new frame even
        }

        //if (!scene) scene = new SceneDemo(&(gEngine.getD3D()), mm, tm);

        input.Frame(); // read input and update state

        gEngine.getCamera()->perFrameUpdate(timer.sincePrev(), input); // move the camera 
        // XXX should the above update be somewhere specific?

        soundSystem.perFrameUpdate();

        scene->update((float)timer.sinceInit(), (float)timer.sincePrev());  // in case you actually want to implement a game, the gameplay would happen in this update() call

        if (!gEngine.RenderScene(timer, scene.get())) return -1;
            
        //Sleep(1); // for luck? seems to have no effect on a moderm multi-core PC really.

        angle += (float)(M_PI) * (float)timer.sincePrev();
        test = angle;

        static double last_honked = 0.0;
        if (input.IsPressed(DIK_E) && timer.sinceInit() - last_honked > 0.5)
        {
            soundSystem.play(beepverb); // just a test of the sound system
            last_honked = timer.sinceInit();
        }


        if (input.IsPressed(DIK_ESCAPE)) done = true;

        if (input.IsPressed(DIK_LALT) && input.IsPressed(DIK_F4)) done = true; // be nice

        timer.Sample(); // read timer and update variables
    }

    scene = nullptr;
    lua_close(L);

    DestroyWindow(window);
    UnregisterClass(L"MainClass", progInstance);

    return 0;
}

