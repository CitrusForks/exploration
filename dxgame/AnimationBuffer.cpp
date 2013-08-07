#include "stdafx.h"
#include "AnimationBuffer.h"

#include <DirectXMath.h>
#include <dxgiformat.h>

using namespace std;
using namespace DirectX;


AnimationBuffer::AnimationBuffer(void) : m_texture(nullptr), m_view(nullptr)
{
}


AnimationBuffer::~AnimationBuffer(void)
{
}


static void loadAiQ(XMFLOAT4& dest, aiQuaternion &src)
{
    dest.x = src.x;
    dest.y = src.y;
    dest.z = src.z;
    dest.w = src.w;
}

void AnimationBuffer::load( const aiScene *scene, ID3D11Device *dev )
{
    double maxTick = 0;

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
    vector<XMFLOAT4> buffer;
    unsigned yStride = (int)maxTick+1;
    unsigned zStride = yStride * scene->mAnimations[0]->mNumChannels;
    buffer.resize(zStride * 3);
    
    for (int keyType = rotation; keyType < scaling; ++keyType)
    {
        for (unsigned bone = 0; bone < scene->mAnimations[0]->mNumChannels; ++bone)
        {
            aiNodeAnim *anim = scene->mAnimations[0]->mChannels[bone];

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
                    XMStoreFloat4(&buffer[keyType * zStride + bone * yStride + aTime], a);
                } else
                {

                    assert(floor(aTime) == aTime);
                    assert(floor(bTime) == bTime);

                    for (double t = aTime; t < bTime; ++t)
                    {
                        XMStoreFloat4(&buffer[0 + bone * yStride + t], XMVectorLerp(a, b, (t - aTime) / (bTime - aTime)));
                    }
                }
            }
            // at this point aTime is the time of the last key in the channel; fill in the rest of the texture line with the value a if necessary
            if (aTime < maxTick)
            {
                for (double t = aTime; t < maxTick; ++t)
                {
                    XMStoreFloat4(&buffer[0 + bone * yStride + t], a);
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

    CD3D11_TEXTURE3D_DESC texDesc(DXGI_FORMAT_R32G32B32A32_FLOAT, maxTick + 1, scene->mAnimations[0]->mNumChannels, 3, 1, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_IMMUTABLE);
    D3D11_SUBRESOURCE_DATA subResource = { &buffer[0], yStride, zStride };
    HRESULT rc = dev->CreateTexture3D(&texDesc, &subResource, &m_texture);
    if (FAILED(rc))
    {
        throw ("Could not create 3D texture for animation buffer.");
    }

    CD3D11_SHADER_RESOURCE_VIEW_DESC srvd(m_texture, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 1);
    dev->CreateShaderResourceView(m_texture, &srvd, &m_view);


}


void AnimationBuffer::release()
{
    if (m_view) m_view->Release();
    if (m_texture) m_texture->Release();
}

void AnimationBuffer::setAsResource( ID3D11DeviceContext *ctx )
{
    ctx->VSSetShaderResources(0xB, 1, &m_view);
}
