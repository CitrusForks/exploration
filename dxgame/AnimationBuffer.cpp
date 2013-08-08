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
    dest.w = src.w;
}

void AnimationBuffer::load( const aiScene *scene, ID3D11Device *dev )
{
    maxTick = 0;

    for (unsigned i = 0; i < scene->mNumAnimations; ++i)
    {
        cout << "Animation " << i << endl;
        cout << "mNumChannels = " << scene->mAnimations[i]->mNumChannels << endl;
        cout << "mNumMeshChannels = " << scene->mAnimations[i]->mNumMeshChannels << endl; // not actually used in liba.i.
        for (unsigned j = 0; j < scene->mAnimations[i]->mNumChannels; ++j)
        {
            aiNodeAnim *anim = scene->mAnimations[i]->mChannels[j];
            cout << "Channel: " << anim->mNodeName.C_Str() << ": ";
            cout << anim->mNumRotationKeys << ", " << anim->mNumPositionKeys << ", " << anim->mNumScalingKeys << endl;

            maxTick = max(maxTick, max(anim->mRotationKeys[anim->mNumRotationKeys-1].mTime, max(anim->mPositionKeys[anim->mNumPositionKeys-1].mTime, anim->mScalingKeys[anim->mNumScalingKeys-1].mTime)));
        }
        break; // only support 1 animation now since that's all we've got in our object(s); animations of different actions just start at particular frames
    }

    assert(floor(maxTick) == maxTick);

    assert(sizeof(XMFLOAT4) == sizeof(aiQuaternion));
    assert(sizeof(XMFLOAT3) == sizeof(aiVector3D));

    // we need a texture to populate
    m_ySize = (unsigned int)maxTick + 1;
    unsigned yStride = m_xSize = scene->mAnimations[0]->mNumChannels;
    m_zStride = m_xSize * m_ySize;
    m_buffer.resize(m_zStride * 3);
    
    for (int keyType = 0; keyType < 3; ++keyType)
    {
        for (unsigned bone = 0; bone < scene->mAnimations[0]->mNumChannels; ++bone)
        {
            aiNodeAnim *anim = scene->mAnimations[0]->mChannels[bone];

            assert(m_animationNodes.find(anim->mNodeName.C_Str()) == m_animationNodes.end() || m_animationNodes[anim->mNodeName.C_Str()] == bone);
            m_animationNodes[anim->mNodeName.C_Str()] = bone; // save the node in a way that's easy to look up

            unsigned keyNum;
            switch(keyType)
            {
            case rotation: keyNum = anim->mNumRotationKeys; break;
            case translation: keyNum = anim->mNumPositionKeys; break;
            case scaling: keyNum = anim->mNumScalingKeys; break;
            }

            XMVECTOR a = XMVectorSet(0,0,0,0), b = XMVectorSet(0,0,0,0);
            double aTime = 0, bTime = 0;

            for (unsigned k = 0; k < keyNum; ++k)
            {
                switch(keyType)
                {
                case rotation:
                    a = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&anim->mRotationKeys[k].mValue));
                    aTime = anim->mRotationKeys[k].mTime;

                    if (k == keyNum - 1) break;

                    b = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&anim->mRotationKeys[k+1].mValue));
                    bTime = anim->mRotationKeys[k+1].mTime;

                    break;

                case translation:
                    a = XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&anim->mPositionKeys[k].mValue));
                    aTime = anim->mPositionKeys[k].mTime;

                    if (k == keyNum - 1) break;

                    b = XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&anim->mPositionKeys[k+1].mValue));
                    bTime = anim->mPositionKeys[k+1].mTime;

                    break;

                case scaling:
                    a = XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&anim->mScalingKeys[k].mValue));
                    aTime = anim->mScalingKeys[k].mTime;

                    if (k == keyNum - 1) break;

                    b = XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&anim->mScalingKeys[k+1].mValue));
                    bTime = anim->mScalingKeys[k+1].mTime;

                    break;

                }

                if (k == keyNum-1)
                {
                    XMStoreFloat4(&m_buffer[keyType * m_zStride + yStride * (int)aTime + bone], a);
                } else
                {

                    assert(floor(aTime) == aTime);
                    assert(floor(bTime) == bTime);

                    for (double t = aTime; t < bTime; ++t)
                    {
                        XMStoreFloat4(&m_buffer[keyType * m_zStride + yStride * (int)t + bone], XMVectorLerp(a, b, (float)((t - aTime) / (bTime - aTime))));
                    }
                }
            }
            // at this point aTime is the time of the last key in the channel; fill in the rest of the texture column with the value `a` if necessary
            if (aTime < maxTick)
            {
                for (double t = aTime; t < maxTick; ++t)
                {
                    XMStoreFloat4(&m_buffer[keyType * m_zStride + yStride * (int)t + bone], a);
                }
            }
        }
    }
#if 0
    for (int i = 0; i < scene->mAnimations[0]->mChannels[0]->mNumRotationKeys; ++i)
    {
        cout << scene->mAnimations[0]->mChannels[0]->mRotationKeys[i].mTime << ", ";
    }
    cout << endl;
#endif

    CD3D11_BUFFER_DESC bufDesc(MAX_BONES * 3 * sizeof(float) * 4, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
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

    animationTick = rand() % (int)maxTick;

    D3D11_MAPPED_SUBRESOURCE sub;

    HRESULT hr = ctx->Map(m_bones, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
    if (FAILED(hr)) throw("seriously? can't map subresource?");

    for (int i = 0; i < 3; ++i)
    {
        memcpy((char*)sub.pData + MAX_BONES * i * (sizeof(float) * 4), &m_buffer[i * m_zStride + m_xSize * (int)animationTick], m_xSize * sizeof(float) * 4);
    }
    ctx->Unmap(m_bones, 0);

    ctx->VSSetConstantBuffers(0xB, 1, &m_bones); // B is for bones!
}
