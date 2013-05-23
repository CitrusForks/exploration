// This loader reads a 3D object with libassimp and dumps it all into one indexed buffer. 
// That saves on draw calls but makes it impossible to set materials per mesh. Instead, 
// materials would have to be sent with every vertex, approximately doubling the memory 
// required for vertex data. 

#include "StdAfx.h"
#include "CompoundMesh.h"

#include <math.h>
#include <assert.h>

#include <unordered_map>
#include <vector>
#include <stack>
#include <functional>

#include <locale>
#include <codecvt>
#include <iomanip>


#include "vertex.h"

#include <assimp/types.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


// this demonstrates the use of for_each(), lambda, and std::function<> ... but doesn't justify it
// the lambda is the part that looks like: [capturedvars] (params) { statements; }
// probably don't write this:
void CompoundMesh::WalkNodes(CompoundMeshNode &node, std::function<void (CompoundMeshNode &)> f)
{
    f(node); // apply f() to node
    for_each(node.children.begin(), node.children.end(), [this, f] (CompoundMeshNode &n) { WalkNodes(n, f); } ); // recurse for all children
}
// but it was fun to write so there's that at least



CompoundMesh::CompoundMesh(void) : m_aiScene(nullptr)
{
}


CompoundMesh::~CompoundMesh(void)
{
    if (m_aiScene) aiReleaseImport(m_aiScene);

    WalkNodes(m_root, [] (CompoundMeshNode &node)
    {
        for_each(node.meshes.begin(), node.meshes.end(), [] (SimpleMesh &mesh)
        {
            // for_each() abuse here is just to try it out; maybe C++11 is cool? who knows?
            mesh.Release();
        });
    });

    for (auto i = m_textureReference.begin(); i != m_textureReference.end(); ++i) (*i).second.Shutdown();
    // writing the above loop with for_each() makes it more verbose and less readable :/

    m_textureReference.clear();
}


// load a modle via libassimp and store separate meshes in a tree structure that mirros aiScene
bool CompoundMesh::load(ID3D11Device* device, ID3D11DeviceContext *devCtx, char *modelFileName)
{
    m_aiScene = aiImportFile(modelFileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded); // load meshes and make sure they're suitable for drawing
    // for a production release, it would be best to ensure the meshes are correct on disk and avoid massaging them into shape here; impact on load time is uncertain but could be dire

    if (nullptr == m_aiScene)
    {
        Errors::Cry("Could not load model ", modelFileName);
        return false;
    }

    // populate vertices and indices with data from m_aiScene
    recursive_interleave(device, devCtx, m_aiScene->mRootNode, m_root);

    int indexCount = 0;

    WalkNodes(m_root, [&indexCount] (CompoundMeshNode &l_node)
    {
        for (auto i = l_node.meshes.begin(); i != l_node.meshes.end(); ++i) indexCount += i->getIndexCount();
    });

    std::cout << modelFileName << " loaded with " << indexCount << " total indices." << std::endl;

    return true;
}


#if 0
// from example code: http://assimp.svn.sourceforge.net/viewvc/assimp/trunk/samples/SimpleOpenGL/Sample_SimpleOpenGL.c?revision=1332&content-type=text%2Fplain
// TODO: update to use xnamath/d3dmath
void CompoundMesh::get_bounding_box_for_node (const aiNode* nd, 
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
void CompoundMesh::get_bounding_box (aiVector3D* min, aiVector3D* max)
{
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(m_aiScene->mRootNode,min,max,&trafo);
}
#endif // 0



void CompoundMesh::apply_material(SimpleMesh::Material *to, aiMaterial *mtl)
{
    assert(sizeof(XMFLOAT4) == sizeof(aiColor4D)); // they're both four floats packed into one structure
    aiColor4D color;
    float shinines, shininessStrength;
    unsigned int max; // not used?

    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &color)) 
        memcpy(&to->diffuse, &color, sizeof(XMFLOAT4));
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &color)) 
        memcpy(&to->ambient, &color, sizeof(XMFLOAT4));
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &color)) 
        memcpy(&to->specular, &color, sizeof(XMFLOAT4));
    if(AI_SUCCESS == aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shinines, &max)
       && AI_SUCCESS == aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &shininessStrength, &max))
        to->shininess = shinines * shininessStrength;

}



bool CompoundMesh::recursive_interleave( ID3D11Device* device, ID3D11DeviceContext *devCtx, const struct aiNode *nd, CompoundMeshNode &node )
{
    assert(sizeof(XMFLOAT4X4) == sizeof(aiMatrix4x4));

    XMMATRIX localTransform = XMLoadFloat4x4((XMFLOAT4X4*) &(nd->mTransformation));

    // update transform
    // TODO XXX
    // probably calls for a separate stream with a local-space transformation quaternion per vertex
#if 0
    aiTransposeMatrix4(&m);
    glPushMatrix();
    glMultMatrixf((float*)&m);
#endif

    std::vector<Vertex> vertices;  // declared here to avoid reallocating them for each mesh; it's easy enough to reuse them
    std::vector<unsigned> indices;

    // iterate over meshes in current node
    for (unsigned n = 0; n < nd->mNumMeshes; ++n) 
    {
        const aiMesh* mesh = m_aiScene->mMeshes[nd->mMeshes[n]];

        SimpleMesh interleavedMesh;

        unsigned texNum = 0;

        // store the material properties
        aiMaterial *mat = m_aiScene->mMaterials[mesh->mMaterialIndex];
        apply_material(&interleavedMesh.m_material, mat);
        

        // get the correct diffuse component texture, load it if necessary
        aiString texPath;
        aiReturn rc = m_aiScene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath); // texPath is in utf8
        if (texPath.length)
        {
            char *c_path = (char*)texPath.C_Str(); 
            int c_len = texPath.length;  // not including terminating \0
            while (*c_path == '\\' || *c_path == '/' || *c_path == '.') { ++c_path; --c_len; } // cut off any path stuff
            int length = MultiByteToWideChar(CP_UTF8, 0, c_path, c_len + 1, 0, 0);  // get length of wchar_t result
            std::unique_ptr<wchar_t>wPath(new wchar_t[length]);  // allocate buffer
            MultiByteToWideChar(CP_UTF8, 0, c_path, c_len + 1, wPath.get(), length); // get the path in wchar_t
            wstring path(wPath.get());  // and wstring

            auto reference = m_textureReference.find(path);  // have we loaded it already?

            if (reference == m_textureReference.end())
            {
                // texture not found, load it
                std::cout << "Loading new texture: " << c_path << std::endl;

                LoadedTexture newTex;
                if (!newTex.Initialize(device, devCtx, wPath.get()))
                {
                    Errors::Cry("Failed to load texture", c_path);
                } else
                {
                    interleavedMesh.m_material.diffuseTexture = m_textureReference[path] = newTex; // success
                }
            } else
            {
                interleavedMesh.m_material.diffuseTexture = reference->second; // already loaded! 
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
        vertices.clear();
        vertices.reserve(mesh->mNumVertices); // allocated required memory ahead of time!
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
        indices.clear();
        indices.reserve(mesh->mNumFaces * 3); // triangles, of course! the only consequence of a mistake here would be a slight performanec loss due to reallocations
        for (unsigned t = 0; t < mesh->mNumFaces; ++t) 
        {
            const struct aiFace* face = &mesh->mFaces[t];

            if (face->mNumIndices != 3) continue; // we're only drawing triangles
            // The load flags are set for triangulating complex polygons. Points (value 1) and lines (value 2 ) are of dubious utility for this game engine.

            for (unsigned i = 0; i < face->mNumIndices; ++i)
            {
                indices.push_back(face->mIndices[i]);
            }
        }

        // feed the vertex and index buffers to the GPU
        D3D11_BUFFER_DESC vbufDesc = { vertices.size() * sizeof(Vertex), D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0};
        D3D11_SUBRESOURCE_DATA vbufSub = { vertices.data(), 0, 0 };

        HRESULT hr = device->CreateBuffer(&vbufDesc, &vbufSub, &interleavedMesh.m_vertexBuffer);
        if (FAILED(hr))
        {
            Errors::Cry("CreateBuffer() failed for vertex buffer.");            
            return false;
        }

        D3D11_BUFFER_DESC ibufDesc = { indices.size() * sizeof(unsigned), D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0, 0, 0};
        D3D11_SUBRESOURCE_DATA ibufSub = { indices.data(), 0, 0 };

        device->CreateBuffer(&ibufDesc, &ibufSub, &interleavedMesh.m_indexBuffer);
        if (FAILED(hr))
        {
            Errors::Cry("CreateBuffer() failed for index buffer.");
        }

        interleavedMesh.m_indexCount = indices.size();

        interleavedMesh.loaded = true;

        if (interleavedMesh.m_indexCount == 0) continue; // :/

        node.meshes.push_back(interleavedMesh);
    }


    // process all child nodes
    for (unsigned n = 0; n < nd->mNumChildren; ++n) 
    {
        CompoundMeshNode child;
        if (!recursive_interleave(device, devCtx, nd->mChildren[n], child)) return false;
        node.children.push_back(child);
    }

    return true;
}

#if 0
// recursively render meshes for all nodes
bool CompoundMesh::Render( ID3D11DeviceContext *deviceContext, VanillaShaderClass *shader, FXMVECTOR cameraPosition, CXMMATRIX worldMatrix, CXMMATRIX viewMatrix, CXMMATRIX projectionMatrix, CompoundMeshNode *node /*= nullptr */ )
{
    bool rc = true; // return value

    WalkNodes(m_root, [&deviceContext, &shader, &time, &lightDirection, &rc, &cameraPosition, &worldMatrix, &viewMatrix, &projectionMatrix] (CompoundMeshNode &node) // capturing an *XMVECTOR variable is a bad idea, doesn't work
    {
        // auto deviceContext = in_deviceContext; // C++ lambda wart: can't capture a variable that's only in scope due to being captured by the containing lambda; must make a local copy
        //auto shader = in_shader; // maybe C++ lambdas aren't good :|
        for (auto mesh = node.meshes.begin(), end = node.meshes.end(); mesh != end; ++mesh)
        {
            if (mesh->m_indexBuffer == nullptr || mesh->m_vertexBuffer == nullptr)
            {
                std::cerr << "strange, empty mesh";
                continue;
            }

            mesh->setBuffers(deviceContext);

            SimpleMesh::Material &mat = mesh->m_material;
            if (!shader->SetPSMaterial(deviceContext, mat.ambient, mat.diffuse, mat.shininess, mat.specular) ||
                !shader->Render(deviceContext, mesh->getIndexCount(), worldMatrix, viewMatrix, projectionMatrix, XMVectorZero(), mesh->m_material.diffuseTexture.GetTexture()))
            {
                rc = false;
                break;
            }
        }
    });

    return rc;
}
#else

// recursively render meshes for all nodes
// lambda-free version
bool CompoundMesh::Render( ID3D11DeviceContext *deviceContext, VanillaShaderClass *shader, 
    FXMVECTOR cameraPosition, 
    CXMMATRIX worldMatrix, CXMMATRIX viewMatrix, CXMMATRIX projectionMatrix, CompoundMeshNode *node /* = nullptr */ )
{
    if (!node) node = &m_root;

    for (auto mesh = node->meshes.begin(), end = node->meshes.end(); mesh != end; ++mesh)
    {
        if (mesh->m_indexBuffer == nullptr || mesh->m_vertexBuffer == nullptr || !mesh->getIndexCount())
        {
            std::cerr << "strange, empty mesh";
            continue;
        }

        mesh->setBuffers(deviceContext); // point the GPU at the right geometry data

        SimpleMesh::Material &mat = mesh->m_material;
        if (!shader->SetPSMaterial(deviceContext, mat.ambient, mat.diffuse, mat.shininess, mat.specular))
        {
            return false;
        }

        if (!shader->Render(deviceContext, mesh->getIndexCount(), worldMatrix, viewMatrix, projectionMatrix, cameraPosition, mesh->m_material.diffuseTexture.GetTexture()))
        {
            return false;
        }
    }

    for (auto i = node->children.begin(); i != node->children.end(); ++i)
    {
        if (!Render(deviceContext, shader, cameraPosition, worldMatrix, viewMatrix, projectionMatrix, &(*i))) return false;
    }

    return true;
}
#endif