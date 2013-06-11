#ifndef INTERMEDIATERENDERTARGET_H
#define INTERMEDIATERENDERTARGET_H

#include <d3d11.h>

class IntermediateRenderTarget
{
private:
    ID3D11Texture2D *m_texture;
    ID3D11Texture2D *m_resolvedTexture;
    ID3D11ShaderResourceView *m_resourceView;
    ID3D11RenderTargetView *m_targetView;

    int m_width, m_height;

public:
    IntermediateRenderTarget(ID3D11Device *dev, ID3D11DeviceContext *devCtx, int width, int height);
    
    IntermediateRenderTarget();
    ~IntermediateRenderTarget(void);

    // should be clear; this sets m_targetView as the current render target
    void setAsRenderTarget(ID3D11DeviceContext *devCtx, ID3D11DepthStencilView *realDepthBuffer);

    // returns m_resourceView for use as a texture source
    ID3D11ShaderResourceView **getResourceViewAndResolveMSAA(ID3D11DeviceContext *devCtx);

    // clear the buffer with 0x00000000
    void clear(ID3D11DeviceContext *devCtx);

    // release the texture
    void Shutdown();
};

#endif
