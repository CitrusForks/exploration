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


bool ComplexMesh::load(char *modelFileName, ID3D11Device* device)
{
    m_aiScene = aiImportFile(modelFileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_MakeLeftHanded);

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


void ComplexMesh::get_bounding_box (aiVector3D* min, aiVector3D* max)
{
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(m_aiScene->mRootNode,min,max,&trafo);
}