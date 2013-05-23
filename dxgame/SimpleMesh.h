#ifndef SIMPLEMESH_H
#define SIMPLEMESH_H

#include <xnamath.h>
#include <vector>
#include <d3d11.h>
#include "vertex.h"
#include "vanillashaderclass.h"

class CompoundMesh;
class LoadedTexture;

class SimpleMesh
{
protected: // we're going to abuse SimpleMesh as a glorified struct in CompoundMesh; it's not good for much on its own.
    struct Material
    {
        XMFLOAT4 ambient;  // multiply light components by these...
        XMFLOAT4 diffuse;  //   ""
        XMFLOAT4 specular; //   ""
        float shininess;  // specular exponent
        LoadedTexture diffuseTexture; // the bog standard texture
        LoadedTexture normalMap;      // wishful thinking? not implemented yet
        LoadedTexture specularMap;    // need this for characters to look correct
    } m_material;

    bool loaded;
    ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
    unsigned int m_indexCount;

    friend CompoundMesh;

public:
    void setBuffers(ID3D11DeviceContext *deviceContext);
    bool load(wchar_t *objFileName, ID3D11Device* device, XMFLOAT2 texture_scaler = XMFLOAT2(1.0f, 1.0f));
    void Release();

    unsigned int getIndexCount() { return m_indexCount; }


    SimpleMesh();
    ~SimpleMesh(void);
};

#endif