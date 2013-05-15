#include "stdafx.h"


#include "SimpleMesh.h"
#include "objloader.hpp"
#include "vboindexer.hpp"
#include <vector>
// wat


bool SimpleMesh::load(wchar_t *objFileName, ID3D11Device* device, XMFLOAT2 texture_scaler)
{
	if (loaded) return false;
        std::vector<Vertex> temp_verts;   // potentially unindexed vertex data ends up here
	loadOBJ(objFileName, temp_verts); 
        printf("From %d verts loaded, ", temp_verts.size());

        std::vector<unsigned short> indices; // indexes end up here
        std::vector<Vertex> vertices;        // unique vertices end up here
        indexVBO(temp_verts, indices, vertices);
	printf("%d indices, %d vertices\n", indices.size(), vertices.size());

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

        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
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
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
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

SimpleMesh::SimpleMesh() : loaded(false)
{
}

SimpleMesh::SimpleMesh(wchar_t *objFN)
{
	load(objFN);
}

SimpleMesh::~SimpleMesh(void)
{
}

void SimpleMesh::render(CXMMATRIX model_to_world_space)
{
	glEnable(GL_TEXTURE_2D);

	if (tex)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);		
	}

	glm::dmat4 model = model_to_world_space;

	glm::mat4 MVP;
	MVP << (perspective * view * model);

	glm::mat4 MV;
	MV << (view * model);

	glm::mat4 M;
	M << model;

	glm::mat4 V;
	V << view;

	glm::mat3 N(V * M); // normal matrix	
	// N = glm::transpose(glm::inverse(N)); // ref: http://www.arcsynthesis.org/gltut/Illumination/Tut09%20Normal%20Transformation.html	

	//NaNtest(perspective);
	//NaNtest(view);
	//NaNtest(model);
	//NaNtest(MVP);

	glm::vec4 test(0.0, 0.0, 0.0, 1.0);
	test = MVP * test;
	//printf("%lf, %lf, %lf, %lf\n", test[0], test[1], test[2], test[3]);

	glErrorCheck();

	glUseProgram(idProgram); glErrorCheck();

	glUniformMatrix4fv(idMVP, 1, 0, &MVP[0][0]); glErrorCheck();
	glUniformMatrix4fv(idMV, 1, 0, &MV[0][0]); glErrorCheck();
	glUniformMatrix3fv(idN, 1, 0, &N[0][0]); glErrorCheck();
	glUniform1i(idTex, 0); glErrorCheck();	

	glUniformMatrix4fv(glGetUniformLocation(idProgram, "M"), 1, 0, &M[0][0]); glErrorCheck();
	glUniformMatrix4fv(glGetUniformLocation(idProgram, "V"), 1, 0, &V[0][0]); glErrorCheck();

	glUniform3fv(idEye, 1, &eye[0]); glErrorCheck();

	glBindVertexArray(VAO_id); glErrorCheck();

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);
	glUseProgram(0); glErrorCheck();
}

