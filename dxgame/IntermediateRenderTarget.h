#ifndef INTERMEDIATERENDERTARGET_H
#define INTERMEDIATERENDERTARGET_H

#include <d3d11.h>

class IntermediateRenderTarget
{
private:
    ID3D11Texture2D *m_texture;
    ID3D11ShaderResourceView *m_resourceView;
    ID3D11RenderTargetView *m_targetView;

public:
    IntermediateRenderTarget(ID3D11Device *dev, ID3D11DeviceContext *devCtx, int width, int height);
    ~IntermediateRenderTarget(void);
    void setAsRenderTarget(ID3D11DeviceContext *devCtx, ID3D11DepthStencilView *realDepthBuffer);
    ID3D11ShaderResourceView *getResourceView();
    void clear(ID3D11DeviceContext *devCtx);
};

#endif
