#include "stdafx.h"
#include "AnimationBuffer.h"

#include <DirectXMath.h>
#include <dxgiformat.h>
#include <stdlib.h>

using namespace std;
using namespace DirectX;


AnimationBuffer::AnimationBuffer(void) : m_bones(nullptr)
{
}


AnimationBuffer::~AnimationBuffer(void)
{
}


static void loadAiQ(XMFLOAT4 &dest, aiQuaternion &src)
{
    dest.x = src.x;
    dest.y = src.y;
    dest.z = src.z;
    dest.w = src.w; // load so component names are preserved rather than order in memory, fwiw
}



void AnimationBuffer::load( const aiScene *scene, ID3D11Device *dev )
{
    maxTick = 0;

    m_aiScene = scene;

    for (unsigned i = 0; i < scene->mNumAnimations; ++i)
    {
        cout << "Animation " << i << endl;
        cout << "mNumChannels = " << scene->mAnimations[i]->mNumChannels << endl;
        cout << "mNumMeshChannels = " << scene->mAnimations[i]->mNumMeshChannels << endl; // not actually used in liba.i.
        for (unsigned j = 0; j < scene->mAnimations[i]->mNumChannels; ++j)
        {
            aiNodeAnim *anim = scene->mAnimations[i]->mChannels[j];
            m_animationNodes[anim->mNodeName.C_Str()] = j;
            cout << "Channel: " << anim->mNodeName.C_Str() << ": ";
            cout << anim->mNumRotationKeys << ", " << anim->mNumPositionKeys << ", " << anim->mNumScalingKeys << endl;

            maxTick = max(maxTick, max(anim->mRotationKeys[anim->mNumRotationKeys-1].mTime, max(anim->mPositionKeys[anim->mNumPositionKeys-1].mTime, anim->mScalingKeys[anim->mNumScalingKeys-1].mTime)));
        }
        break; // only support 1 animation now since that's all we've got in our object(s); animations of different actions just start at particular frames
    }

    assert(floor(maxTick) == maxTick);

    assert(sizeof(XMFLOAT4) == sizeof(aiQuaternion));
    assert(sizeof(XMFLOAT3) == sizeof(aiVector3D));

    CD3D11_BUFFER_DESC bufDesc(MAX_BONES * sizeof(XMFLOAT4X4) + sizeof(XMFLOAT4X4), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    dev->CreateBuffer(&bufDesc, NULL, &m_bones);
}


void AnimationBuffer::release()
{
    if (m_bones) m_bones->Release();
}





void AnimationBuffer::updateResource( ID3D11DeviceContext *ctx, double animationTick )
{
    // TODO: interpolation. Get the basics working first!

    if (animationTick < 0 || animationTick > maxTick) return;

    D3D11_MAPPED_SUBRESOURCE sub;

    HRESULT hr = ctx->Map(m_bones, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
    if (FAILED(hr)) throw("seriously? can't map subresource?");
     
    XMFLOAT4X4 *data = (XMFLOAT4X4*)sub.pData;

    // update bones here!
    for (unsigned b = 0; b < m_aiScene->mAnimations[0]->mNumChannels; ++b)
    {
        getBoneTransform(&data[b], b, animationTick, true);
    }

    ctx->Unmap(m_bones, 0);

    ctx->VSSetConstantBuffers(0xB, 1, &m_bones); // B is for bones!
}


void AnimationBuffer::getNodeTransform( DirectX::XMFLOAT4X4 *dest, std::string bone, double animationTick )
{
    auto iter = m_animationNodes.find(bone);

    if (iter == m_animationNodes.end()) return;

    int i = iter->second;

    XMFLOAT4 quat, tran, scal;
    getBoneTransform(dest, i, animationTick);
}


void AnimationBuffer::getBoneTransform( DirectX::XMFLOAT4X4 *transform, int bone, double animationTick, bool transpose /*= false*/ )
{
    XMFLOAT4 rotation;
    XMFLOAT3 translation, scaling;

    aiNodeAnim *anim = m_aiScene->mAnimations[0]->mChannels[bone];

    unsigned k;

    // TODO: interpolation

    for (k = 0; k < anim->mNumRotationKeys - 1; ++k) // -1 because we always accept the last item in the array (N.B., occasionally the arrays are of length 1)
    {
        if (anim->mRotationKeys[k].mTime <= animationTick && anim->mRotationKeys[k+1].mTime > animationTick) break;
    }
    loadAiQ(rotation, anim->mRotationKeys[k].mValue);

    for (k = 0; k < anim->mNumPositionKeys - 1; ++k)
    {
        if (anim->mPositionKeys->mTime <= animationTick && anim->mRotationKeys[k+1].mTime > animationTick) break;
    }
    translation.x = anim->mPositionKeys[k].mValue.x;
    translation.y = anim->mPositionKeys[k].mValue.y;
    translation.z = anim->mPositionKeys[k].mValue.z;

    for (k = 0; k < anim->mNumScalingKeys - 1; ++k)
    {
        if (anim->mScalingKeys->mTime <= animationTick && anim->mScalingKeys[k+1].mTime > animationTick) break;
    }
    scaling.x = anim->mScalingKeys[k].mValue.x;
    scaling.y = anim->mScalingKeys[k].mValue.y;
    scaling.z = anim->mScalingKeys[k].mValue.z;



    XMMATRIX M = XMMatrixMultiply(XMMatrixRotationQuaternion(XMLoadFloat4(&rotation)), XMMatrixScaling(scaling.x, scaling.y, scaling.z));

    //XMStoreFloat4x4(transform, XMMatrixMultiply(XMLoadFloat4x4(transform), XMMatrixScaling(scaling.x, scaling.y, scaling.z)));

    M.r[3].m128_f32[0] = translation.x;
    M.r[3].m128_f32[1] = translation.y;
    M.r[3].m128_f32[2] = translation.z;

/*
    transform->_41 = translation.x;
    transform->_42 = translation.y;
    transform->_43 = translation.z;
*/
    if (transpose) M = XMMatrixTranspose(M);

    XMStoreFloat4x4(transform, M);
}
