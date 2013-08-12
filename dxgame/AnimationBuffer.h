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

    std::unordered_map<std::string, int> m_animationNodes; // maps bone name to its index in the data buffer (the 3d texture)

public:
    void load(const aiScene *scene, ID3D11Device *dev);
    void release();

    void updateResource( ID3D11DeviceContext *ctx, double animationTick );

    // note, bone names and node names are one and the same
    int getBoneNum(const char *bone) 
    {
        int i = m_animationNodes[bone]; 
        return i;
    }
    bool loaded() { return m_bones != nullptr; }

    void getNodeTransform(DirectX::XMFLOAT4X4 *dest, std::string bone, double animationTick);

    AnimationBuffer(void);
    ~AnimationBuffer(void);
};

