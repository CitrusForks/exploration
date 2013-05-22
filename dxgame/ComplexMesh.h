#ifndef COMPLEXMESH_H
#define COMPLEXMESH_H

#include <assimp/types.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include "vanillashaderclass.h"

class ComplexMesh
{
private:
    bool loaded;
    ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
    LoadedTexture *m_Texture;
    unsigned int m_indexCount;

    const aiScene *m_aiScene;

    void get_bounding_box_for_node (const aiNode* nd, aiVector3D* min, aiVector3D* max, aiMatrix4x4* trafo);
    void get_bounding_box (aiVector3D* min, aiVector3D* max);

    void recursive_render (ID3D11DeviceContext *deviceContext, VanillaShaderClass *shader, const struct aiScene *sc, const struct aiNode *nd);

public:
    void setBuffers(ID3D11DeviceContext *deviceContext);
    bool load(char *modelFileName, ID3D11Device* device, Assimp::Importer &importer);

    bool Render(VanillaShaderClass *shader);

    ComplexMesh();
    ~ComplexMesh(void);
};

#endif