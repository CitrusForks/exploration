// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <algorithm>
#include <codecvt>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>
#include <memory>
#include <string>
#include <string>
#include <unordered_map>
#include <vector>

#include <stdio.h>
#include <string.h>
#include <tchar.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <dxgi.h>
#include <d3d11.h>

// external lib deps:
#include <lua.hpp>
#include <fmod.hpp>

#include <assimp/types.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// mm, obsolete D3DX11 library stuff...
#include <d3dx10math.h>
#include <d3dx11async.h>
//#include <d3dx11Effect.h>

#include <xnamath.h>

#include "d3dclass.h"
#include "LoadedTexture.h"
#include "WICTextureLoader.h"
#include "vertex.h"
#include "Chronometer.h"
#include "Sound.h"
#include "inputclass.h"
#include "FirstPerson.h"
#include "IntermediateRenderTarget.h"
#include "Errors.h"
#include "SimpleText.h"
#include "CompoundMesh.h"
#include "ComplexMesh.h"
#include "SimpleMesh.h"

#include "vanillashaderclass.h"

