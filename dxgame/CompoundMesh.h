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
#include <memory>

class CompoundMesh
{
private:
    bool loaded;
    struct CompoundMeshNode
    {
        std::vector<SimpleMesh> meshes;

        std::vector<std::shared_ptr<CompoundMeshNode>> children;

        DirectX::XMFLOAT4X4 localTransform; 
        DirectX::XMFLOAT4X4 globalTransform; // for scene graph, replaced by node animations when applicable?

        std::string name; // to look up a transform to replace the above
        int boneIndex; // to look up a transform even faster than by name :P
    };

    shared_ptr<CompoundMeshNode> m_root;

    unordered_map <std::string, std::shared_ptr<CompoundMeshNode>> m_nodeByName;

    DirectX::BoundingOrientedBox m_bBox;
    DirectX::BoundingSphere m_bSphere;

    const aiScene *m_aiScene;

    TextureManager *m_textureManager;

    AnimationBuffer m_animation;

    void WalkNodes(std::shared_ptr<CompoundMesh::CompoundMeshNode> node, std::function<void (CompoundMesh::CompoundMeshNode &)> f);

    void get_bounding_box_for_node (const aiNode* nd, aiVector3D* min, aiVector3D* max, aiMatrix4x4* trafo);
    void get_bounding_box (aiVector3D* min, aiVector3D* max);

    bool recursive_interleave( ID3D11Device* device, ID3D11DeviceContext *devCtx, const struct aiNode *nd, CompoundMeshNode &node, DirectX::CXMMATRIX parentTransform );
    void apply_material(SimpleMesh::Material *to, aiMaterial *mtl);
    
    void updateNodeTransforms(double animationTick, CompoundMeshNode *node = nullptr, DirectX::CXMMATRIX parentTransform = DirectX::XMMatrixIdentity());
    void reindexNodes(std::shared_ptr<CompoundMeshNode> start);

    DirectX::XMMATRIX getNodeGlobalTransform(std::string nodename);


public:
    bool load(ID3D11Device* device, ID3D11DeviceContext *devCtx, TextureManager *texman, char *modelFileName);
    bool render( ID3D11DeviceContext *deviceContext, VanillaShaderClass *shader, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, DirectX::CXMMATRIX projectionMatrix, std::vector<Light> &lights, bool orthoProjection = false, double animationTick = 1.0, CompoundMeshNode *node = nullptr, DirectX::CXMMATRIX parentNodeTransform = DirectX::XMMatrixIdentity());
    void release();


    CompoundMesh();
    ~CompoundMesh(void);
};

#endif