#ifndef COMPOUNDMESH_H
#define COMPOUNDMESH_H

#include <assimp/types.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include "vertex.h"
#include "SimpleMesh.h"

#include "vanillashaderclass.h"

class CompoundMesh
{
private:
    bool loaded;
    struct CompoundMeshNode
    {
        std::vector<SimpleMesh> meshes;

        std::vector<CompoundMeshNode> children;
    } m_root;

    const aiScene *m_aiScene;

    std::unordered_map<wstring, LoadedTexture> m_textureReference;

    void WalkNodes(CompoundMesh::CompoundMeshNode &node, std::function<void (CompoundMesh::CompoundMeshNode &)> f);

    void get_bounding_box_for_node (const aiNode* nd, aiVector3D* min, aiVector3D* max, aiMatrix4x4* trafo);
    void get_bounding_box (aiVector3D* min, aiVector3D* max);

    bool recursive_interleave( ID3D11Device* device, ID3D11DeviceContext *devCtx, const struct aiNode *nd, CompoundMeshNode &node );
    
public:
    bool load(ID3D11Device* device, ID3D11DeviceContext *devCtx, char *modelFileName);
    bool Render( ID3D11DeviceContext *deviceContext, VanillaShaderClass *shader, 
        XMFLOAT3 &lightDirection, float time, FXMVECTOR cameraPosition, 
        CXMMATRIX worldMatrix, CXMMATRIX viewMatrix, CXMMATRIX projectionMatrix );


    CompoundMesh();
    ~CompoundMesh(void);
};

#endif