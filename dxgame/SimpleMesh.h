#ifndef SIMPLEMESH_H
#define SIMPLEMESH_H

#include <directxmath.h>
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include "vertex.h"
#include "vanillashaderclass.h"
#include "LoadedTexture.h"

struct SimpleMesh
{
//protected: // we're going to abuse SimpleMesh as a glorified struct in CompoundMesh; it's not good for much on its own.
    struct Material
    {
        DirectX::XMFLOAT4 ambient;  // multiply light components by these...
        DirectX::XMFLOAT4 diffuse;  //   ""
        DirectX::XMFLOAT4 specular; //   ""
        float shininess;  // specular exponent
        LoadedTexture diffuseTexture; // the bog standard texture
        LoadedTexture normalMap;      // wishful thinking? not implemented yet
        LoadedTexture specularMap;    // need this for characters to look correct

        Material() : // I want defaults that are easy to test with...
            ambient(0.1f, 0.1f, 0.1f, 1.0f), diffuse(0.5f, 0.5f, 0.5f, 1.0f), specular(1.0f, 1.0f, 1.0f, 1.0f), shininess(100)
        {
        }
    } m_material;

    bool loaded;
    ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
    ID3D11Texture2D *m_boneData;
    unsigned int m_indexCount;

    void setBuffers(ID3D11DeviceContext *deviceContext);
    bool load(wchar_t *objFileName, ID3D11Device* device, DirectX::XMFLOAT2 texture_scaler = DirectX::XMFLOAT2(1.0f, 1.0f));
    void Release();

    unsigned int getIndexCount() { return m_indexCount; }


    SimpleMesh();
    ~SimpleMesh(void);
};

#endif