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
    ID3D11ShaderResourceView *m_view;

    std::unordered_map<const char *, int> m_animationNodes; // maps bone name to its index in the data buffer (the 3d texture)

public:
    void load(const aiScene *scene, ID3D11Device *dev);
    void release();

    void setAsResource(ID3D11DeviceContext *ctx);

    int getBoneNum(const char *bone) { return m_animationNodes[bone]; }
    bool loaded() { return m_view != nullptr; }

    AnimationBuffer(void);
    ~AnimationBuffer(void);
};

