#ifndef COMPLEXMESH_H
#define COMPLEXMESH_H

#include <assimp/types.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <vector>
#include <string>
#include <unordered_map>
#include "vertex.h"

class VanillaShaderClass;
class LoadedTexture;

class ComplexMesh
{
private:
    bool loaded;
    ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
    unsigned int m_indexCount;

    const aiScene *m_aiScene;

    std::vector<LoadedTexture> m_textures;
    std::vector<ID3D11ShaderResourceView*> m_resourceViews; // shortcut for the data in the above array; this can be passed directly to shaders
    std::unordered_map<std::wstring, unsigned> m_textureReference;

    void get_bounding_box_for_node (const aiNode* nd, aiVector3D* min, aiVector3D* max, aiMatrix4x4* trafo);
    void get_bounding_box (aiVector3D* min, aiVector3D* max);

    void recursive_interleave(ID3D11Device* device, ID3D11DeviceContext *devCtx, std::vector<Vertex> &vertices, std::vector<unsigned> &indices, const struct aiScene *sc, const struct aiNode *nd);

public:
    void setBuffers(ID3D11DeviceContext *deviceContext);
    bool load(ID3D11Device* device, ID3D11DeviceContext *devCtx, char *modelFileName);

    unsigned int getIndexCount() { return m_indexCount; }
    ID3D11ShaderResourceView **getShaderResourceViews() { return m_resourceViews.data(); }
    unsigned int getShaderResourceViewCount() { return m_resourceViews.size(); } 

    ComplexMesh();
    ~ComplexMesh(void);
};

#endif