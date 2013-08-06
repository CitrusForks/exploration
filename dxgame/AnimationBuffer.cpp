#include "stdafx.h"
#include "AnimationBuffer.h"

#include <DirectXMath.h>

using namespace std;
using namespace DirectX;


AnimationBuffer::AnimationBuffer(void) : m_texture(nullptr)
{
}


AnimationBuffer::~AnimationBuffer(void)
{
}


void AnimationBuffer::load( aiScene *scene )
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

            m_animationNodes[anim->mNodeName.C_Str()] = anim; // save the node in a way that's easy to look up :P
        }
        break; // only support 1 animation now since that's all we've got in our object(s); animations of different actions just start at particular frames
    }

    assert(floor(maxTick) == maxTick);

    // we need a texture to populate
    vector<XMFLOAT4> buffer;
    buffer.resize((int)maxTick * scene->mAnimations[0]->mNumChannels);
#if 0
    for (unsigned j = 0; j < scene->mAnimations[0]->mNumChannels; ++j)
    {
        aiNodeAnim *anim = scene->mAnimations[0]->mChannels[j];
        anim->mRotationKeys[0].mTime; // ???
    }
#endif
    for (int i = 0; i < scene->mAnimations[0]->mChannels[0]->mNumRotationKeys; ++i)
    {
        cout << scene->mAnimations[0]->mChannels[0]->mRotationKeys[i].mTime << ", ";
    }
    cout << endl;

}


void AnimationBuffer::release()
{
    if (m_texture) m_texture->Release();
}
