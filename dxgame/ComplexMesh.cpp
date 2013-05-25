// This loader reads a 3D object with libassimp and dumps it all into one indexed buffer. 
// That saves on draw calls but makes it impossible to set materials per mesh. Instead, 
// materials would have to be sent with every vertex, approximately doubling the memory 
// required for vertex data. 

#include "StdAfx.h"
#include "ComplexMesh.h"

#include <math.h>
#include <assert.h>

#include <unordered_map>
#include <vector>

#include <locale>
#include <codecvt>
#include <iomanip>


#include "vertex.h"

#include <assimp/types.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


ComplexMesh::ComplexMesh(void) : m_aiScene(nullptr)
{
}


ComplexMesh::~ComplexMesh(void)
{
    if (m_indexBuffer) m_indexBuffer->Release();
    if (m_vertexBuffer) m_vertexBuffer->Release();

    if (m_aiScene) aiReleaseImport(m_aiScene);

    for (auto i = m_textures.begin(); i != m_textures.end(); ++i) (*i).shutdown();
    m_textures.clear();
    m_resourceViews.clear();
}


bool ComplexMesh::load(ID3D11Device* device, ID3D11DeviceContext *devCtx, char *modelFileName)
{
    m_aiScene = aiImportFile(modelFileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded); // load meshes and make sure they're suitable for drawing
    // for a production release, it would be best to ensure the meshes are correct on disk and avoid massaging them into shape here; impact on load time is uncertain, could be dire

    if (nullptr == m_aiScene)
    {
        Errors::Cry("Could not load model ", modelFileName);
        return false;
    }

    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;

    // populate vertices and indices with data from m_aiScene
    recursive_interleave(device, devCtx, vertices, indices, m_aiScene, m_aiScene->mRootNode);

    // feed the vertex and index buffers to the GPU
    D3D11_BUFFER_DESC vbufDesc = { vertices.size() * sizeof(Vertex), D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0};
    D3D11_SUBRESOURCE_DATA vbufSub = { vertices.data(), 0, 0 };

    HRESULT hr = device->CreateBuffer(&vbufDesc, &vbufSub, &m_vertexBuffer);
    if (FAILED(hr))
    {
        Errors::Cry("CreateBuffer() failed for vertex buffer.");
        return false;
    }

    D3D11_BUFFER_DESC ibufDesc = { indices.size() * sizeof(unsigned), D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0, 0, 0};
    D3D11_SUBRESOURCE_DATA ibufSub = { indices.data(), 0, 0 };

    device->CreateBuffer(&ibufDesc, &ibufSub, &m_indexBuffer);
    if (FAILED(hr))
    {
        Errors::Cry("CreateBuffer() failed for index buffer.");
        return false;
    }

    m_indexCount = indices.size();
    return true;
}


void ComplexMesh::setBuffers(ID3D11DeviceContext *deviceContext)
    {
    // four buffers, 
    unsigned stride = sizeof(Vertex);
    unsigned offset = 0;

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


// from example code: http://assimp.svn.sourceforge.net/viewvc/assimp/trunk/samples/SimpleOpenGL/Sample_SimpleOpenGL.c?revision=1332&content-type=text%2Fplain
// TODO: update to use xnamath/d3dmath
void ComplexMesh::get_bounding_box_for_node (const aiNode* nd, 
	aiVector3D* min, 
	aiVector3D* max, 
	aiMatrix4x4* trafo
){
	aiMatrix4x4 prev;
	unsigned int n = 0, t;

	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = m_aiScene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp,trafo);

			min->x = min(min->x,tmp.x);
			min->y = min(min->y,tmp.y);
			min->z = min(min->z,tmp.z);

			max->x = max(max->x,tmp.x);
			max->y = max(max->y,tmp.y);
			max->z = max(max->z,tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}

// gets a bounding box... could be useful perhaps?
// don't call it repeatedly; cache the results
void ComplexMesh::get_bounding_box (aiVector3D* min, aiVector3D* max)
{
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(m_aiScene->mRootNode,min,max,&trafo);
}


void ComplexMesh::recursive_interleave(ID3D11Device *device, ID3D11DeviceContext *devCtx, std::vector<Vertex> &vertices, std::vector<unsigned> &indices, const struct aiScene *sc, const struct aiNode *nd)
{
    aiMatrix4x4 m = nd->mTransformation;

    // update transform
    // TODO XXX
    // probably calls for a separate stream with a local-space transformation quaternion per vertex
#if 0
    aiTransposeMatrix4(&m);
    glPushMatrix();
    glMultMatrixf((float*)&m);
#endif

    // draw all meshes assigned to this node
    for (unsigned n = 0; n < nd->mNumMeshes; ++n) 
    {
        const aiMesh* mesh = m_aiScene->mMeshes[nd->mMeshes[n]];

        int baseIndex = vertices.size(); // we're pouring all the vertices into one array but mesh indexes keep restarting at 0; add this to make sure they're pointing to the right area

        unsigned texNum = 0;

        //apply_material(sc->mMaterials[mesh->mMaterialIndex]);

        //if (sc->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_BLEND_FUNC, 

        // get the correct diffuse component texture, load it if necessary
        aiString texPath;
        aiReturn rc = sc->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath); // texPath is in utf8
        if (texPath.length)
        {
            char *c_path = (char*)texPath.C_Str();
            int c_len = texPath.length;
            while (*c_path == '\\' || *c_path == '/' || *c_path == '.') { ++c_path; --c_len; }
            int length = MultiByteToWideChar(CP_UTF8, 0, c_path, c_len + 1, 0, 0);
            std::unique_ptr<wchar_t>wPath(new wchar_t[length]);
            MultiByteToWideChar(CP_UTF8, 0, c_path, c_len + 1, wPath.get(), length); // get the path in wchar_t
            wstring path(wPath.get());  // and wstring

            auto reference = m_textureReference.find(path);

            if (reference == m_textureReference.end())
            {
                std::cout << "Loading new texture: " << c_path << std::endl;
                // texture not found, load it
                LoadedTexture newTex;
                if (!newTex.initialize(device, devCtx, wPath.get()))
                {
                    Errors::Cry("Failed to load texture", c_path);
                } else
                {
                    texNum = m_textureReference[path] = m_textures.size();
                    m_resourceViews.push_back(*(newTex.getTexture()));
                    m_textures.push_back(newTex);
                }
            } else
            {
                texNum = (*reference).second; // already loaded!
            }
        }

#if 0
        if(mesh->mNormals == NULL) {
            glDisable(GL_LIGHTING);
        } else {
            glEnable(GL_LIGHTING);
        }
#endif

        // copy vertex coordinates, normals, and texture coordinates for every vertex in this mesh
        for (unsigned i = 0; i < mesh->mNumVertices; ++i)
        {
            Vertex v;
                
            assert(sizeof(XMFLOAT3) == sizeof(aiVector3D));

            v.pos = reinterpret_cast<XMFLOAT3*>(mesh->mVertices)[i]; // the data in aiVector3D is just three floats, just like XMFLOAT3
            v.normal = reinterpret_cast<XMFLOAT3*>(mesh->mNormals)[i];
            v.tex0.x = mesh->mTextureCoords[0][i].x; // AssImp always stores texture coordinates in a 3-member vector while we're using 2 floats
            v.tex0.y = mesh->mTextureCoords[0][i].y;
            v.texArray = texNum;

            vertices.push_back(v);
        }


        // for all triangles, add indices to the index buffer
        for (unsigned t = 0; t < mesh->mNumFaces; ++t) 
        {
            const struct aiFace* face = &mesh->mFaces[t];

            if (face->mNumIndices != 3) continue; // we're only drawing triangles. 
            // The load flags are set for triangulating complex polygons. Points (value 1) and lines (value 2 ) are of dubious utility for this game engine.

            for (unsigned i = 0; i < face->mNumIndices; ++i)
            {
                indices.push_back(face->mIndices[i] + baseIndex);
            }
        }

    }

    // process all child nodes
    for (unsigned n = 0; n < nd->mNumChildren; ++n) 
    {
        recursive_interleave(device, devCtx, vertices, indices, sc, nd->mChildren[n]);
    }
}