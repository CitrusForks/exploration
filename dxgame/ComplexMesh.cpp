#include "StdAfx.h"
#include "ComplexMesh.h"

#include <math.h>

#include <assimp/types.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


ComplexMesh::ComplexMesh(void)
{
}


ComplexMesh::~ComplexMesh(void)
{
}


bool ComplexMesh::load(char *modelFileName, ID3D11Device* device, Assimp::Importer &importer)
{
    m_aiScene = importer.ReadFile(modelFileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_MakeLeftHanded);

    return m_aiScene != nullptr;
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


void ComplexMesh::recursive_render( ID3D11DeviceContext *deviceContext, VanillaShaderClass *shader, const struct aiScene *sc, const struct aiNode *nd )
{
    unsigned int i;
    unsigned int n = 0, t;
    aiMatrix4x4 m = nd->mTransformation;

    // update transform
    // TODO XXX
#if 0
    aiTransposeMatrix4(&m);
    glPushMatrix();
    glMultMatrixf((float*)&m);
#endif

    // draw all meshes assigned to this node
    for (; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = m_aiScene->mMeshes[nd->mMeshes[n]];

        //apply_material(sc->mMaterials[mesh->mMaterialIndex]);

#if 0
        if(mesh->mNormals == NULL) {
            glDisable(GL_LIGHTING);
        } else {
            glEnable(GL_LIGHTING);
        }
#endif
        for (t = 0; t < mesh->mNumFaces; ++t) {
            const struct aiFace* face = &mesh->mFaces[t];
#if 0

            GLenum face_mode;

            switch(face->mNumIndices) {
            case 1: face_mode = GL_POINTS; break;
            case 2: face_mode = GL_LINES; break;
            case 3: face_mode = GL_TRIANGLES; break;
            default: face_mode = GL_POLYGON; break;
            }

            glBegin(face_mode);

#endif
            if (face->mNumIndices != 3) continue; // we're only drawing triangles. 
            // The load flags are set for triangulating complex polygons. Points and lines are of dubious utility for this game engine.

#if 0
            for(i = 0; i < face->mNumIndices; i++) {
                int index = face->mIndices[i];
                if(mesh->mColors[0] != NULL)
                    glColor4fv((GLfloat*)&mesh->mColors[0][index]);
                if(mesh->mNormals != NULL) 
                    glNormal3fv(&mesh->mNormals[index].x);
                glVertex3fv(&mesh->mVertices[index].x);
            }

            glEnd();
#endif
     
            //mesh->mColors;

            // four buffers, 
            unsigned int stride[4] = { sizeof(aiVector3D), sizeof(aiVector3D), sizeof(aiVector3D), sizeof(aiVector3D) }; // position, normal, uv0, uv1 (just reserving a spot for now)
            unsigned int offset[4] = {0, 0, 0, 0};

            // Set the vertex buffer to active in the input assembler so it can be rendered.
            deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, stride, offset);

            // Set the index buffer to active in the input assembler so it can be rendered.
            deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

            // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
            deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



        }

    }

    // draw all children
    for (n = 0; n < nd->mNumChildren; ++n) {
        recursive_render(deviceContext, shader, sc, nd->mChildren[n]);
    }

    //glPopMatrix();
}