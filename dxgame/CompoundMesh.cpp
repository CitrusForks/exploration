// This loader reads a 3D object with libassimp and duplicates its mesh tree with SimpleMesh objects.
// This seems like the conventional approach. It should make it easier to implement animation.

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

#include <DirectXCollision.h>

using namespace std;
using namespace DirectX;

// from the net somewhere, modified
BOOL FileExistsW(wchar_t *path)
{
    DWORD dwAttrib = GetFileAttributesW(path);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}



// this demonstrates the use of for_each(), lambda, and function<> ... but doesn't justify it
// the lambda is the part that looks like: [capturedvars] (params) { statements; }
// maybe don't write this:
void CompoundMesh::WalkNodes(CompoundMeshNode &node, function<void (CompoundMeshNode &)> f)
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
}



void CompoundMesh::release()
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
}



// load a model via libassimp and store separate meshes in a tree structure that mirrors aiScene
bool CompoundMesh::load(ID3D11Device* device, ID3D11DeviceContext *devCtx, TextureManager *texman, char *modelFileName)
{
    assert(texman);
    m_textureManager = texman;
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
        for (auto &i: l_node.meshes) indexCount += i.getIndexCount();
    });

    cout << modelFileName << " loaded with " << indexCount << " total indices." << endl;

    aiVector3D min, max;

    get_bounding_box(&min, &max);

    bBoxMin.x = min.x; // too tired to make this copy clever right now; it's not important.
    bBoxMin.y = min.y;
    bBoxMin.z = min.z;
    bBoxMin.w = 1;

    bBoxMax.x = max.x;
    bBoxMax.y = max.y;
    bBoxMax.z = max.z;
    bBoxMax.w = 1;

    m_bBox.CreateFromPoints(m_bBox, XMLoadFloat4(&bBoxMin), XMLoadFloat4(&bBoxMax));

    return true;
}


#if 1
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
    
    if(AI_SUCCESS == aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shinines, &max))
    {
        to->shininess = shinines;
        if (shinines < 0.001)
        {
            cout << "Oddity: near-zero specular exponent." << endl;
            to->shininess = 100;
        }
    } else
    {
        cout << "Oddity: no specular power found." << endl;
        to->shininess = 100;
        //to->specular = XMFLOAT4(0, 0, 0, 1);
    }


    if(AI_SUCCESS == aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &shininessStrength, &max))
        to->specular.x *= shininessStrength, to->specular.y *= shininessStrength, to->specular.z *= shininessStrength;

}



bool CompoundMesh::recursive_interleave( ID3D11Device* device, ID3D11DeviceContext *devCtx, const struct aiNode *nd, CompoundMeshNode &node )
{
    bool uvWarningPrinted = false;
    bool tangentWarningPrinted = false;

    assert(sizeof(XMFLOAT4X4) == sizeof(aiMatrix4x4));

    XMMATRIX localTransform = XMLoadFloat4x4((XMFLOAT4X4*) &(nd->mTransformation));

    // update transform
    // TODO XXX
    // probably calls for a separate stream with a local-space transformation quaternion per vertex? also for more reading...
#if 0
    aiTransposeMatrix4(&m);
    glPushMatrix();
    glMultMatrixf((float*)&m);
#endif

    vector<Vertex> vertices;  // declared here to avoid reallocating them for each mesh; it's easy enough to reuse them
    vector<unsigned> indices;

    // iterate over meshes in current node
    for (unsigned n = 0; n < nd->mNumMeshes; ++n) 
    {
        const aiMesh* mesh = m_aiScene->mMeshes[nd->mMeshes[n]];

        SimpleMesh interleavedMesh;

        // store the material properties
        aiMaterial *mat = m_aiScene->mMaterials[mesh->mMaterialIndex];
        apply_material(&interleavedMesh.m_material, mat);
        

        // get the correct diffuse component texture, load it if necessary
        aiString texPath;
        aiReturn rc = m_aiScene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath); // texPath is in utf8
        if (texPath.length)
        {   
            // there's a texture! retrieve it via the texture manager
            if (!m_textureManager->getTextureUTF8(device, devCtx, (char*)texPath.C_Str(), texPath.length, interleavedMesh.m_material.diffuseTexture)) return false;
        }

        // same method gets us a normal map
        aiString normalPath;
        rc = m_aiScene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DISPLACEMENT, 0, &normalPath); // texPath is in utf8
        if (!normalPath.length)
        {
            // normal map not specified... try searching for it blindly, due to bad .mtl/.dae files with Chekov?
            char *w = strstr((char*)texPath.C_Str(), "_D");
            if (w)
            {
                w[1] = 'N';
                wstring path = m_textureManager->utf8ToWstring((char*)texPath.C_Str(), texPath.length);
                if (FileExistsW((wchar_t*)path.c_str()))
                {
                    if (!m_textureManager->getTexture(path, device, devCtx, interleavedMesh.m_material.normalMap)) return false;
                }
				w[1] = 'D';
            }
        } else
        {   
            if (!m_textureManager->getTextureUTF8(device, devCtx, (char*)texPath.C_Str(), texPath.length, interleavedMesh.m_material.normalMap)) return false;
        }

		// repeat for a specular map
		aiString specularPath;
		rc = m_aiScene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_SPECULAR, 0, &specularPath); // texPath is in utf8
		if (!specularPath.length)
		{
			// specular map not specified... try searching for it blindly, due to bad .mtl/.dae files with Chekov?
			char *w = strstr((char*)texPath.C_Str(), "_D");
			if (w)
			{
				w[1] = 'S';
				wstring path = m_textureManager->utf8ToWstring((char*)texPath.C_Str(), texPath.length);
				if (FileExistsW((wchar_t*)path.c_str()))
				{
					if (!m_textureManager->getTexture(path, device, devCtx, interleavedMesh.m_material.specularMap)) return false;
				}
				w[1] = 'D';
			}
		} else
		{   
			if (!m_textureManager->getTextureUTF8(device, devCtx, (char*)specularPath.C_Str(), texPath.length, interleavedMesh.m_material.specularMap))
			{
				// don't quit if we can't find a specular map; most likely nobody cares all that much
				cout << "Warning, missing specular map: " << specularPath.C_Str() << endl;
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
        // also tangents
        vertices.clear();
        vertices.reserve(mesh->mNumVertices); // allocated required memory ahead of time!
        for (unsigned i = 0; i < mesh->mNumVertices; ++i)
        {
            Vertex v;
                
            assert(sizeof(XMFLOAT3) == sizeof(aiVector3D));

            v.pos = reinterpret_cast<XMFLOAT3*>(mesh->mVertices)[i]; // the data in aiVector3D is just three floats, just like XMFLOAT3
            v.normal = reinterpret_cast<XMFLOAT3*>(mesh->mNormals)[i];

            if (nullptr == mesh->mTextureCoords[0]) // try to copy uv texture coordinates
            {
                v.tex0.x = v.tex0.y = 0;
                if (!uvWarningPrinted)
                {
                    cerr << "Missing texture coordinates... " << endl;
                    uvWarningPrinted = true;
                }
            } else
            {
                v.tex0.x = mesh->mTextureCoords[0][i].x; // AssImp always stores texture coordinates in a 3-member vector while we're using 2 floats
                v.tex0.y = mesh->mTextureCoords[0][i].y;
            }

            if (nullptr == mesh->mTangents)
            {
                v.tangent = XMFLOAT3(0,0,0);
            } else
            {
                v.tangent = reinterpret_cast<XMFLOAT3*>(mesh->mTangents)[i];
            }

            v.texArray = 0;

            vertices.push_back(v);
        }


        // for all triangles, add indices to the index buffer
        indices.clear();
        if (mesh->mNumFaces < 1)
        {
            cerr << "Oddity: no faces in mesh." << endl;
            continue;
        }
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

        if (vertices.size() < 1) 
        {
            cerr << "Oddity: no vertices in mesh. Perhaps it was composed entirely of non-triangles?";
            continue;
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

        if (indices.size() < 1)
        {
            cerr << "Oddity: no indices in mesh!";
            continue;
        }

        D3D11_BUFFER_DESC ibufDesc = { indices.size() * sizeof(unsigned), D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0, 0, 0};
        D3D11_SUBRESOURCE_DATA ibufSub = { indices.data(), 0, 0 };

        hr = device->CreateBuffer(&ibufDesc, &ibufSub, &interleavedMesh.m_indexBuffer);
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
bool CompoundMesh::render( ID3D11DeviceContext *deviceContext, VanillaShaderClass *shader, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, DirectX::CXMMATRIX projectionMatrix, std::vector<Light> &lights, CompoundMeshNode *node /*= nullptr */ )
{
    bool rc = true; // return value

    WalkNodes(m_root, [&deviceContext, &shader, &rc, &worldMatrix, &viewMatrix, &projectionMatrix] (CompoundMeshNode &node) // capturing an *XMVECTOR variable is a bad idea, doesn't work
    {
        // auto deviceContext = in_deviceContext; // C++ lambda wart: can't capture a variable that's only in scope due to being captured by the containing lambda; must make a local copy
        //auto shader = in_shader; // maybe C++ lambdas aren't good :|
        for (auto mesh = node.meshes.begin(), end = node.meshes.end(); mesh != end; ++mesh)
        {
            if (mesh->m_indexBuffer == nullptr || mesh->m_vertexBuffer == nullptr)
            {
                cerr << "strange, empty mesh";
                continue;
            }

            mesh->setBuffers(deviceContext);

            SimpleMesh::Material &mat = mesh->m_material;
            bool useNormalMap = mat.normalMap.getTexture() ? true : false;
            if (!shader->SetPSMaterial(deviceContext, mat.ambient, mat.diffuse, mat.shininess, mat.specular, useNormalMap) ||
                !shader->Render(deviceContext, mesh->getIndexCount(), worldMatrix, viewMatrix, projectionMatrix, mesh->m_material.normalMap.getTexture(), mesh->m_material.diffuseTexture.getTexture()))
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
bool CompoundMesh::render( ID3D11DeviceContext *deviceContext, VanillaShaderClass *shader, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, 
    DirectX::CXMMATRIX projectionMatrix, vector<Light> &lights, CompoundMeshNode *node /*= nullptr */ )
{
    if (!node)
    {
        node = &m_root;

        // clip to view frustum
        BoundingFrustum frustum(projectionMatrix); // TODO this should be moved somewhere higher up if the engine ever becomes CPU-bound
        frustum.Transform(frustum, XMMatrixInverse(nullptr, viewMatrix)); // move the frustum as though it was an object within the world; probably slow?

        XMFLOAT3 center;
        float radius = 1.0f; // XXX incorrect, calculate this somewhere

        XMStoreFloat3(&center, XMVector3Transform(XMVectorZero(), worldMatrix));

        BoundingSphere bSphere(center, radius);

        BoundingBox bBox(m_bBox);
        bBox.Transform(bBox, worldMatrix);

        if (!frustum.Contains(bBox) && !frustum.Intersects(bBox)) return true;
    }

    for (auto mesh = node->meshes.begin(), end = node->meshes.end(); mesh != end; ++mesh)
    {
        if (mesh->m_indexBuffer == nullptr || mesh->m_vertexBuffer == nullptr || !mesh->getIndexCount())
        {
            cerr << "strange, empty mesh";
            continue;
        }

        mesh->setBuffers(deviceContext); // point the GPU at the right geometry data

        SimpleMesh::Material &mat = mesh->m_material;
        bool useNormalMap = mat.normalMap.getTexture() ? true : false;
		bool useSpecularMap = mat.specularMap.getTexture() ? true : false;
        if (!shader->SetPSMaterial(deviceContext, mat.ambient, mat.diffuse, mat.shininess, mat.specular, useNormalMap, useSpecularMap))
        {
            return false;
        }

        if (!shader->Render(deviceContext, mesh->getIndexCount(), worldMatrix, viewMatrix, projectionMatrix, mesh->m_material.normalMap.getTexture(), mesh->m_material.specularMap.getTexture(), &lights, mesh->m_material.diffuseTexture.getTexture()))
        {
            return false;
        }
    }

    for (auto i = node->children.begin(); i != node->children.end(); ++i)
    {
        if (!render(deviceContext, shader, worldMatrix, viewMatrix, projectionMatrix, lights, &(*i))) return false;
    }

    return true;
}

#endif