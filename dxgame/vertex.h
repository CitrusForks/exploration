#ifndef VERTEX_H
#define VERTEX_H

#include <directxmath.h>
#include <string.h> // for memcmp()

// Vertex matches the format of the vertex buffer
struct Vertex
{ 
	DirectX::XMFLOAT3 pos;    // vertex position in model space
	DirectX::XMFLOAT3 normal; // yep, normal
	DirectX::XMFLOAT2 tex0;   // texture sampler, possibly an array
	DirectX::XMFLOAT2 tex1;   // not used, just experimenting
        unsigned int texArray;  // texture to choose from array if using an array of textures
        DirectX::XMFLOAT3 tangent;// tangent; binormal is calculated in vertex shader
        float boneNum[4];  // index into arrays of translations and rotations (as quaternions); the array should be equivalent to one frame of animation with each member populated from a single aiNodeAnim
        float boneWeights[4];  // bone weights

        Vertex() : pos(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 0.0f), tex0(0.0f, 0.0f), tex1(0.0f, 0.0f), texArray(0)
        {
            for (auto &i : boneNum)
            {
                i = -1;
            }
            ZeroMemory(boneWeights, sizeof(boneWeights));
        }

        // the following operator is meant only to allow the use of Vertex as a key to std::map() and such
	bool operator<(const Vertex that) const{
		return memcmp((void*)this, (void*)&that, (ptrdiff_t)&boneWeights - (ptrdiff_t)&pos)>0;
	};

        void addBoneWeight(int boneIndex, float weight)
        {
            assert((char*)boneWeights == ((char*)boneNum) + 16);
            int j;
            float min = weight;
            for (int i = 0; i < 4; ++i)
            {
                if (boneWeights[i] < min)
                {
                    min = boneWeights[i];
                    j = i;
                }
            }

            if (min >= weight) return; // just keep the most significant bone weights if too many are specified

            boneWeights[j] = weight;
            boneNum[j] = (float)boneIndex;
        }
};





#endif

