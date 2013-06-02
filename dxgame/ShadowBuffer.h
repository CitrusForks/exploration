#pragma once

#include <d3d11.h>

class ShadowBuffer
{
private:
    ID3D11Texture2D *m_texture;
    ID3D11Texture2D *m_depthStencilBuffer;
    ID3D11DepthStencilView *m_depthStencilView;
    ID3D11ShaderResourceView *m_resourceView;

    ID3D11RenderTargetView *m_targetView;


public:
    ShadowBuffer();
    ~ShadowBuffer(void);

    bool init(ID3D11Device *device, ID3D11DeviceContext *deviceContext, int resolutionMultiplier = 1);
    void release();

    // should be clear; this sets m_targetView as the current render target
    void setAsRenderTarget(ID3D11DeviceContext *devCtx);

    // returns m_resourceView for use as a texture source
    ID3D11ShaderResourceView **getResourceView(ID3D11DeviceContext *devCtx);

    // clear the buffer with 0x00000000
    void clear(ID3D11DeviceContext *devCtx);

    // helper method to present an array of these buffers to the GPU
    static void pushToGPU(ID3D11DeviceContext *devCtx, std::vector<ShadowBuffer> &buffers);

    int m_width, m_height;

    static int width, height; // shadow map size defaults? Should these be in Options instead?
};

