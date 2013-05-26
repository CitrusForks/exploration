#include "stdafx.h"


#include "SimpleMesh.h"
#include "objloader.hpp"
#include "vboindexer.hpp"
#include <vector>

using namespace std;
using namespace DirectX;

// load an obj file using a half-baked loader routine; not good for much besides testing and loading monolithic meshes without materials
// CompoundMesh should be a better alternative
bool SimpleMesh::load(wchar_t *objFileName, ID3D11Device* device, XMFLOAT2 texture_scaler)
{
	if (loaded) return false;
        vector<Vertex> temp_verts;   // potentially unindexed vertex data ends up here
	loadOBJ(objFileName, temp_verts); 
        printf("From %d verts loaded, ", temp_verts.size());

        vector<unsigned int> indices; // indexes end up here
        vector<Vertex> vertices;        // unique vertices end up here
        indexVBO(temp_verts, indices, vertices);
	printf("%d indices, %d vertices\n", indices.size(), vertices.size());

        m_indexCount = indices.size();

        if (texture_scaler.x != 1.0f && texture_scaler.y != 1.0f)
	{
            for (auto i = vertices.begin(), end = vertices.end(); i != end; ++i)
	    {
                (*i).tex0.x *= texture_scaler.x; // tile that texture!
                (*i).tex0.y *= texture_scaler.y;
	    }
	}



        // the following is largely copied from modelclass.cpp from the rastertek.com tutorials
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
        D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

        vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertices.size();
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
        vertexData.pSysMem = vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
        result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
        indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        indexBufferDesc.ByteWidth = sizeof(unsigned long) * indices.size();
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
        indexData.pSysMem = indices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	loaded = true;

	return true;

}

SimpleMesh::SimpleMesh() : loaded(false), m_indexCount(0)
{
}




SimpleMesh::~SimpleMesh(void)
{
}


// Set this mesh's buffers as the buffers to render
void SimpleMesh::setBuffers(ID3D11DeviceContext *deviceContext)
{
    // this is basically copied from the rastertek tutorial too

    assert(loaded);
    assert(m_indexBuffer != nullptr);
    assert(m_vertexBuffer!= nullptr);

    // Set vertex buffer stride and offset.
    unsigned stride = sizeof(Vertex); 
    unsigned offset = 0;

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// release resources
void SimpleMesh::Release()
{
    if (!loaded) return;
    m_indexBuffer->Release(); m_indexBuffer = nullptr;
    m_vertexBuffer->Release(); m_vertexBuffer = nullptr;
    loaded = false;
}

