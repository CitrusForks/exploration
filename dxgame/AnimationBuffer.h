#pragma once

#include <d3d11.h>
#include <unordered_map>
#include <vector>

#include <assimp/scene.h>

class AnimationBuffer
{
public:
    enum dataT { rotation = 0, translation = 1, scaling = 2}; // type of transformation

private:
    ID3D11Buffer *m_bones;
    std::vector<DirectX::XMFLOAT4> m_buffer; // due to this, it's best to avoid copying this object; at ~160KB, copying this buffer may or may not matter to performance

    unsigned m_ySize, m_xSize, m_zStride;
    double maxTick;

    std::unordered_map<const char *, int> m_animationNodes; // maps bone name to its index in the data buffer (the 3d texture)

public:
    void load(const aiScene *scene, ID3D11Device *dev);
    void release();

    void updateResource( ID3D11DeviceContext *ctx, double animationTick );

    int getBoneNum(const char *bone) { return m_animationNodes[bone]; }
    bool loaded() { return m_bones != nullptr; }

    AnimationBuffer(void);
    ~AnimationBuffer(void);
};

