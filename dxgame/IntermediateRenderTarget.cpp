#include "StdAfx.h"
#include "IntermediateRenderTarget.h"
#include <iostream>

IntermediateRenderTarget::IntermediateRenderTarget(ID3D11Device *dev, ID3D11DeviceContext *devCtx, int width, int height) : m_texture(nullptr), m_targetView(nullptr), m_resourceView(nullptr)
{
    D3D11_TEXTURE2D_DESC tdesc;
    const DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT; // some weird format with some extra precision?

    tdesc.Width = width;
    tdesc.Height = height;
    tdesc.MipLevels = 1; // no mipmap please
    tdesc.ArraySize = 1;
    tdesc.Format = format; 
    DXGI_SAMPLE_DESC sampleDesc = {1, 0}; // Count, Quality
    tdesc.SampleDesc = sampleDesc;
    tdesc.Usage = D3D11_USAGE_DEFAULT;
    tdesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // the GPU needs to render to the texture and then pass the texture on to a shader
    tdesc.CPUAccessFlags = 0; 
    tdesc.MiscFlags = 0;

    HRESULT rc = dev->CreateTexture2D(&tdesc, nullptr, &m_texture);
    if (FAILED(rc))
    {
        Errors::Cry(L"Couldn't create a texture for off-screen render buffer. :/");
        return;
    }

    D3D11_RENDER_TARGET_VIEW_DESC targetDesc;
    targetDesc.Format = format;
    targetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    targetDesc.Texture2D.MipSlice = 0; // the documentation on this is completely unhelpful but 0 is probably what's needed here

    rc = dev->CreateRenderTargetView(m_texture, &targetDesc, &m_targetView);
    if (FAILED(rc))
    {
        Errors::Cry(L"Couldn't create render target view. :/");
        m_texture->Release();
        return;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
    resourceDesc.Format = format; // the formats must match or the call will fail; which raises the question, why force us to specify it multiple times then?
    resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    resourceDesc.Texture2D.MostDetailedMip = 0; // not sure about this
    resourceDesc.Texture2D.MipLevels = -1; // or this    

    rc = dev->CreateShaderResourceView(m_texture, &resourceDesc, &m_resourceView);
    if (FAILED(rc))
    {
        Errors::Cry(L"Couldn't create shader resource view. :/");
        m_targetView->Release();
        m_texture->Release();
        return;
    }
}


IntermediateRenderTarget::~IntermediateRenderTarget(void)
{
}

void IntermediateRenderTarget::setAsRenderTarget(ID3D11DeviceContext *devCtx, ID3D11DepthStencilView *realDepthBuffer)
{
    devCtx->OMSetRenderTargets(1, &m_targetView, realDepthBuffer);
}

ID3D11ShaderResourceView *IntermediateRenderTarget::getResourceView()
{
    return m_resourceView;
}

void IntermediateRenderTarget::clear(ID3D11DeviceContext *devCtx)
{
    float const blank[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    devCtx->ClearRenderTargetView(m_targetView, blank);
}
