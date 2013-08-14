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

    double maxTick;

    std::unordered_map<std::string, int> m_animationNodes; // maps bone name to its index in aiScene

    const aiScene *m_aiScene;

    void getBoneTransform(DirectX::XMFLOAT4X4 *transform, int bone, double animationTick, bool transpose = false);

public:
    void load(const aiScene *scene, ID3D11Device *dev);
    void release();

    void updateResource( ID3D11DeviceContext *ctx, double animationTick );

    // note, bone names and node names are one and the same
    int getBoneNum(const char *bone) 
    {
        if (m_animationNodes.find(bone) == m_animationNodes.end()) return -1;

        int i = m_animationNodes[bone]; 
        return i;
    }
    bool loaded() { return m_bones != nullptr; }

    void getNodeTransform(DirectX::XMFLOAT4X4 *dest, std::string bone, double animationTick);

    AnimationBuffer(void);
    ~AnimationBuffer(void);
};

