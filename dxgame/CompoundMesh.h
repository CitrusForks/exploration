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
#include "TextureManager.h"

#include "vanillashaderclass.h"

#include <DirectXMath.h>
#include <DirectXCollision.h>

class CompoundMesh
{
private:
    bool loaded;
    struct CompoundMeshNode
    {
        std::vector<SimpleMesh> meshes;

        std::vector<CompoundMeshNode> children;
    } m_root;

    DirectX::BoundingOrientedBox m_bBox;
    DirectX::BoundingSphere m_bSphere;

    const aiScene *m_aiScene;

    TextureManager *m_textureManager;

    AnimationBuffer m_animation;

    void WalkNodes(CompoundMesh::CompoundMeshNode &node, std::function<void (CompoundMesh::CompoundMeshNode &)> f);

    void get_bounding_box_for_node (const aiNode* nd, aiVector3D* min, aiVector3D* max, aiMatrix4x4* trafo);
    void get_bounding_box (aiVector3D* min, aiVector3D* max);

    bool recursive_interleave( ID3D11Device* device, ID3D11DeviceContext *devCtx, const struct aiNode *nd, CompoundMeshNode &node, DirectX::CXMMATRIX parentTransform );
    void apply_material(SimpleMesh::Material *to, aiMaterial *mtl);
    
public:
    bool load(ID3D11Device* device, ID3D11DeviceContext *devCtx, TextureManager *texman, char *modelFileName);
    bool render( ID3D11DeviceContext *deviceContext, VanillaShaderClass *shader, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, DirectX::CXMMATRIX projectionMatrix, std::vector<Light> &lights, bool orthoProjection = false, double animationTick = 1.0, CompoundMeshNode *node = nullptr);
    void release();


    CompoundMesh();
    ~CompoundMesh(void);
};

#endif