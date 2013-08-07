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
        std::vector<float> boneWeights;  // bone weights
        std::vector<unsigned> boneNum;  // index into arrays of translations and rotations (as quaternions); the array should be equivalent to one frame of animation with each member populated from a single aiNodeAnim

        Vertex() : pos(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 0.0f), tex0(0.0f, 0.0f), tex1(0.0f, 0.0f), texArray(0)
        {
            boneWeights.resize(4);
            boneNum.resize(4);
            ZeroMemory(&boneNum[0], sizeof(boneNum));
            ZeroMemory(&boneWeights[0], sizeof(boneWeights));
        }

        // the following operator is meant only to allow the use of Vertex as a key to std::map() and such
	bool operator<(const Vertex that) const{
		return memcmp((void*)this, (void*)&that, (ptrdiff_t)&boneWeights - (ptrdiff_t)&pos)>0;
	};

        void addBoneWeight(int boneIndex, float weight)
        {
            auto v = std::min_element(boneWeights.begin(), boneWeights.end());

            if (*v > weight) return; // just keep the most significant bone weights if too many are specified

            int j = v - boneWeights.begin();            

            boneWeights[j] = weight;
            boneNum[j] = boneIndex;
        }
};





#endif

