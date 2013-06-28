#ifndef CPP_HLSL_DEFS_H
#define CPP_HLSL_DEFS_H

// a file included both in the C++ code and in HLSL to set common compile-time constants

// maximum number of spotlights, needed for sizing arrays:
#define NUM_SPOTLIGHTS 4

// shadow map dimensions
#define SHADOWMAP_DIMENSIONS 128

#define DIRECTIONAL_SHADOW_MULTIPLIER_WIDE 4
#define DIRECTIONAL_SHADOW_MULTIPLIER_LOD1 4

// define this to debug main shaders with VS2012:
//#define DISABLE_OFFSCREEN_BUFFER debug_debug_debug
// 



#endif