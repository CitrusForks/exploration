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

    cout << "maxTick == " << maxTick << endl;

    CD3D11_BUFFER_DESC bufDesc(MAX_BONES * sizeof(XMFLOAT4X4) + sizeof(XMFLOAT4X4), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    dev->CreateBuffer(&bufDesc, NULL, &m_bones);
}


void AnimationBuffer::release()
{
    if (m_bones) m_bones->Release();
}





void AnimationBuffer::updateCurrentBoneKeys( ID3D11DeviceContext *ctx, double animationTick )
{
}


void AnimationBuffer::updateBoneTransforms( ID3D11DeviceContext *ctx, double animationTick, std::vector<DirectX::XMFLOAT4X4> &offsets, std::string currentNode, std::function<DirectX::XMMATRIX (std::string)> f )
{
    XMFLOAT4X4 *data = mapSubresource(ctx);

    if (offsets.size() == 0)
    {
#if 0
        assert(false);
#endif
        return;
    }

    // TODO: interpolation. Get the basics working first!

    //animationTick *= m_aiScene->mAnimations[0]->mTicksPerSecond; // assume the parameter is actually seconds

    if (animationTick < 0 || animationTick > maxTick) return;

    XMMATRIX globalInverseTransform = XMMatrixInverse(nullptr, f(currentNode));

    // update bones here!
    for (unsigned b = 0; b < m_aiScene->mAnimations[0]->mNumChannels && b < offsets.size(); ++b)
    {
        //getBoneTransform(&data[b], b, animationTick, false);
        //XMMATRIX global = XMLoadFloat4x4(&data[b])
        //XMStoreFloat4x4(&data[b], XMMatrixTranspose(XMMatrixMultiply(XMLoadFloat4x4(&offsets[b]), XMLoadFloat4x4(&data[b]))));

        XMMATRIX off = XMLoadFloat4x4(&offsets[b]); 
        
        XMMATRIX global = f(m_aiScene->mAnimations[0]->mChannels[b]->mNodeName.C_Str());

        XMMATRIX M = XMMatrixMultiply(XMMatrixMultiply(off, global), globalInverseTransform);

        XMStoreFloat4x4(&data[b], XMMatrixTranspose(M));
    }

    ctx->Unmap(m_bones, 0);

    ctx->VSSetConstantBuffers(0xB, 1, &m_bones); // B is for bones! NOTE possibly redundant? eliminate?
}




/*
void AnimationBuffer::getNodeTransformByName( DirectX::XMFLOAT4X4 *dest, std::string bone, double animationTick )
{
    auto iter = m_animationNodes.find(bone);

    if (iter == m_animationNodes.end()) return;

    int i = iter->second;

    XMFLOAT4 quat, tran, scal;
    getBoneTransform(dest, i, animationTick);
}
*/


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

    // interpolate! (slerp? or not?)
    if (k < anim->mNumRotationKeys - 1)
    {
        aiQuatKey *k1 = &anim->mRotationKeys[k], *k2 = &anim->mRotationKeys[k+1];
        XMFLOAT4 rot2;
        loadAiQ(rot2, k2->mValue);

        XMVECTOR q = XMQuaternionSlerp(XMLoadFloat4(&rotation), XMLoadFloat4(&rot2), static_cast<float>((animationTick - k1->mTime) / (k2->mTime - k1->mTime)));
        q = XMQuaternionNormalize(q);
        XMStoreFloat4(&rotation, q);
    }


    for (k = 0; k < anim->mNumPositionKeys - 1; ++k)
    {
        if (anim->mPositionKeys[k].mTime <= animationTick && anim->mPositionKeys[k+1].mTime > animationTick) break;
    }
    translation.x = anim->mPositionKeys[k].mValue.x;
    translation.y = anim->mPositionKeys[k].mValue.y;
    translation.z = anim->mPositionKeys[k].mValue.z;

    // interpolate!
    if (k < anim->mNumPositionKeys - 1)
    {
        aiVectorKey *k1 = &anim->mPositionKeys[k], *k2 = &anim->mPositionKeys[k+1];

        XMVECTOR t = XMVectorLerp(XMLoadFloat3(&translation), XMLoadFloat3((XMFLOAT3*)&k2->mValue), static_cast<float>((animationTick - k1->mTime) / (k2->mTime - k1->mTime)));
        XMStoreFloat3(&translation, t);
    }

    for (k = 0; k < anim->mNumScalingKeys - 1; ++k)
    {
        if (anim->mScalingKeys[k].mTime <= animationTick && anim->mScalingKeys[k+1].mTime > animationTick) break;
    }
    scaling.x = anim->mScalingKeys[k].mValue.x;
    scaling.y = anim->mScalingKeys[k].mValue.y;
    scaling.z = anim->mScalingKeys[k].mValue.z;



    XMMATRIX M = XMMatrixRotationQuaternion(XMLoadFloat4(&rotation));

    //assert(scaling.x == 1.0f && scaling.y == 1.0f && scaling.z == 1.0f);

    M = XMMatrixMultiply(M, XMMatrixScaling(scaling.x, scaling.y, scaling.z));

    M = XMMatrixMultiply(M, XMMatrixTranslationFromVector(XMLoadFloat3(&translation)));

/*
    M.r[3].m128_f32[0] = translation.x;
    M.r[3].m128_f32[1] = translation.y;
    M.r[3].m128_f32[2] = translation.z;
*/

    if (transpose) M = XMMatrixTranspose(M);

    XMStoreFloat4x4(transform, M);
}

XMFLOAT4X4 * AnimationBuffer::mapSubresource( ID3D11DeviceContext * ctx )
{
    D3D11_MAPPED_SUBRESOURCE sub;

    HRESULT hr = ctx->Map(m_bones, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
    if (FAILED(hr)) throw("seriously? can't map subresource?");

    XMFLOAT4X4 *data = (XMFLOAT4X4*)sub.pData;

    return data;
}


