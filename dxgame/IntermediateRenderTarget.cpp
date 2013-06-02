#include "StdAfx.h"
#include "IntermediateRenderTarget.h"
#include "Options.h"
#include <iostream>

static const DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT; // some weird format with some extra precision?

IntermediateRenderTarget::IntermediateRenderTarget(ID3D11Device *dev, ID3D11DeviceContext *devCtx, int width, int height) : m_texture(nullptr), m_resolvedTexture(nullptr), m_targetView(nullptr), m_resourceView(nullptr), m_width(width), m_height(height)
{
    D3D11_TEXTURE2D_DESC tdesc;

    tdesc.Width = width; //  * 2; // x2 for supersampling test
    tdesc.Height = height; //  * 2;
    tdesc.MipLevels = 1; // no mipmap please
    tdesc.ArraySize = 1;
    tdesc.Format = format; 
    DXGI_SAMPLE_DESC sampleDesc = {Options::intOptions["MSAACount"], Options::intOptions["MSAAQuality"]}; // Count, Quality
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
    targetDesc.ViewDimension = Options::intOptions["MSAACount"] > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
    targetDesc.Texture2D.MipSlice = 0; // the documentation on this is completely unhelpful but 0 is probably what's needed here

    rc = dev->CreateRenderTargetView(m_texture, &targetDesc, &m_targetView);
    if (FAILED(rc))
    {
        Errors::Cry(L"Couldn't create render target view. :/");
        m_texture->Release();
        return;
    }

    if (Options::intOptions["MSAACount"] > 1)
    {
        // for MSAA, we need a second texture target to which to resolve the multi-sampled output? :/
        tdesc.Width = width; //  * 2; // x2 for supersampling test
        tdesc.Height = height; //  * 2;
        tdesc.MipLevels = 1; // no mipmap please
        tdesc.ArraySize = 1;
        tdesc.Format = format; 
        DXGI_SAMPLE_DESC sampleDesc = {1, 0}; // no multisampling on this target
        tdesc.SampleDesc = sampleDesc;
        tdesc.Usage = D3D11_USAGE_DEFAULT;
        tdesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // the GPU needs to render to the texture and then pass the texture on to a shader
        tdesc.CPUAccessFlags = 0; 
        tdesc.MiscFlags = 0;

        HRESULT rc = dev->CreateTexture2D(&tdesc, nullptr, &m_resolvedTexture);
        if (FAILED(rc))
        {
            Errors::Cry(L"Couldn't create a texture for off-screen render buffer. :/");
            return;
        }
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
    resourceDesc.Format = format; // the formats must match or the call will fail; which raises the question, why force us to specify it multiple times then?
    resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    resourceDesc.Texture2D.MostDetailedMip = 0; // not sure about this
    resourceDesc.Texture2D.MipLevels = -1; // or this    

    rc = dev->CreateShaderResourceView(Options::intOptions["MSAACount"] > 1 ? m_resolvedTexture : m_texture, &resourceDesc, &m_resourceView);
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

// after this call, render calls will draw to our off-screen buffer
void IntermediateRenderTarget::setAsRenderTarget(ID3D11DeviceContext *devCtx, ID3D11DepthStencilView *realDepthBuffer)
{
    // Set up the viewport for rendering at x2 resolution to back buffer
    //D3D11_VIEWPORT viewport = { (float)m_width*2, (float)m_height*2, 0.0f, 1.0f, 0.0f, 0.0f }; // XXX er, this was out of order thanks to bad examples...
    //devCtx->RSSetViewports(1, &viewport);

    devCtx->OMSetRenderTargets(1, &m_targetView, realDepthBuffer); // set our texture as target and that's it!
}

// return resource view and do some configuration for rendering from buffer to swap chain
ID3D11ShaderResourceView ** IntermediateRenderTarget::getResourceViewAndResolveMSAA( ID3D11DeviceContext *devCtx )
{
    // Set up the viewport for rendering at screen resolution
    //D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float)m_width, (float)m_height, 0.0f, 1.0f };
    //devCtx->RSSetViewports(1, &viewport);

    if (Options::intOptions["MSAACount"] > 1) devCtx->ResolveSubresource(m_resolvedTexture, 0, m_texture, 0, format);

    return &m_resourceView;
}

// erase!
void IntermediateRenderTarget::clear(ID3D11DeviceContext *devCtx)
{
    float const blank[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    devCtx->ClearRenderTargetView(m_targetView, blank);
}

// release all the things? 
void IntermediateRenderTarget::Shutdown()
{
    m_targetView->Release();
    m_resourceView->Release();
    m_texture->Release();
    if (m_resolvedTexture) m_resolvedTexture->Release();

    m_targetView = nullptr;
    m_resourceView = nullptr;
    m_texture = nullptr;
    m_resolvedTexture = nullptr;
}
