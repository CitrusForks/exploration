#ifndef SIMPLEMESH_H
#define SIMPLEMESH_H

#include <xnamath.h>
#include <vector>
#include <d3d11.h>
#include "vertex.h"
#include "LoadedTexture.h"
#include "vanillashaderclass.h"

class SimpleMesh
{
private:
    bool loaded;
    ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
    LoadedTexture *m_Texture;
    unsigned int m_indexCount;


public:
    void setBuffers(ID3D11DeviceContext *deviceContext);
    bool load(wchar_t *objFileName, ID3D11Device* device, XMFLOAT2 texture_scaler = XMFLOAT2(1.0f, 1.0f));
    void Release();

    unsigned int getIndexCount() { return m_indexCount; }


    SimpleMesh();
    ~SimpleMesh(void);
};

#endif