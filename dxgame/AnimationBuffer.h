#pragma once

#include <d3d11.h>
#include <unordered_map>
#include <vector>
#include <functional>

#include <assimp/scene.h>

class AnimationBuffer
{
public:
    enum dataT { rotation = 0, translation = 1, scaling = 2}; // type of transformation

    double maxTick;

private:
    ID3D11Buffer *m_bones;

    std::unordered_map<std::string, int> m_animationNodes; // maps bone name to its index in aiScene

    const aiScene *m_aiScene;


public:
    void load(const aiScene *scene, ID3D11Device *dev);
    void release();

    void updateCurrentBoneKeys( ID3D11DeviceContext *ctx, double animationTick );

    DirectX::XMFLOAT4X4 *mapSubresource( ID3D11DeviceContext * ctx );

    void updateBoneTransforms( ID3D11DeviceContext *ctx, double animationTick, std::vector<DirectX::XMFLOAT4X4> &offsets, std::string currentNode, std::function<DirectX::XMMATRIX (std::string)> f );

    // note, bone names and node names are one and the same
    int getBoneNum(const char *bone) 
    {
        if (!bone || *bone == '\0') return -1;
        if (m_animationNodes.find(bone) == m_animationNodes.end()) return -1;

        int i = m_animationNodes[bone]; 
        return i;
    }
    bool loaded() { return m_bones != nullptr; }

    DirectX::XMMATRIX getBoneTransform(DirectX::XMFLOAT4X4 *transform, int bone, double animationTick, bool transpose = false);
    DirectX::XMMATRIX getNodeTransformByName(std::string bone, double animationTick);

    AnimationBuffer(void);
    ~AnimationBuffer(void);
};

