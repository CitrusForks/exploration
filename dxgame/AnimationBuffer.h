#pragma once

#include <d3d11.h>
#include <unordered_map>

#include <assimp/scene.h>

class AnimationBuffer
{
public:
    enum dataT { rotation = 0, translation = 1, scaling = 2}; // type of transformation

private:
    ID3D11Texture3D *m_texture; // X is time in ticks, Y is bone index (think piano roll), Z is data type (see dataT)
    ID3D11Resource *m_view;

    std::unordered_map<const char *, aiNodeAnim *> m_animationNodes; // each one contains an array (one element per frame) of transformation keys for each named node

public:
    void load(aiScene *scene);
    void release();

    AnimationBuffer(void);
    ~AnimationBuffer(void);
};

