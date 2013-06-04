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

#include <directxmath.h>

#include "cpp_hlsl_defs.h"

#include "Actor.h"

#include "LightsAndShadows.h"

#include "Chronometer.h"
#include "CompoundMesh.h"
#include "Errors.h"
#include "FirstPerson.h"
#include "IntermediateRenderTarget.h"

#include "LoadedTexture.h"
#include "ModelManager.h"
#include "Options.h"
#include "TextureManager.h"
#include "SimpleMesh.h"
#include "SimpleText.h"
#include "Sound.h"
#include "WICTextureLoader.h"
#include "d3dclass.h"
#include "Input.h"
#include "vanillashaderclass.h"
#include "vertex.h"

