#include "StdAfx.h"
#include "ShadowBuffer.h"


ShadowBuffer::ShadowBuffer()
{
}

ShadowBuffer::~ShadowBuffer()
{
}


int ShadowBuffer::width = 512;
int ShadowBuffer::height = 512;

bool ShadowBuffer::init( ID3D11Device *device, ID3D11DeviceContext *deviceContext, int resolutionMultiplier /*= 1*/ )
{
    HRESULT rc;

    m_width = width * resolutionMultiplier;
    m_height = height * resolutionMultiplier;

#if 0
    D3D11_TEXTURE2D_DESC tdesc;
    // a render target might be needed in D3D9 but not in modern DirectX since we can just retrieve the depth buffer
    DXGI_FORMAT format = DXGI_FORMAT_A8_UNORM;

    tdesc.Width = width; //  * 2; // x2 for supersampling test
    tdesc.Height = height; //  * 2;
    tdesc.MipLevels = 1; // no mipmap please
    tdesc.ArraySize = 1;
    tdesc.Format = format; // ughhhhhh what do I pick; I think we're not using the output in this buffer at all anyway, it's stupid
    DXGI_SAMPLE_DESC sampleDesc = {1, 0};
    tdesc.SampleDesc = sampleDesc;
    tdesc.Usage = D3D11_USAGE_DEFAULT;
    tdesc.BindFlags = D3D11_BIND_RENDER_TARGET; // | D3D11_BIND_SHADER_RESOURCE;
    tdesc.CPUAccessFlags = 0; 
    tdesc.MiscFlags = 0;

    rc = device->CreateTexture2D(&tdesc, nullptr, &m_texture);
    if (FAILED(rc))
    {
        Errors::Cry(L"Couldn't create a texture for shadow buffer. :/");
        return false;
    }

    D3D11_RENDER_TARGET_VIEW_DESC targetDesc;
    targetDesc.Format = format;
    targetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    targetDesc.Texture2D.MipSlice = 0;

    rc = device->CreateRenderTargetView(m_texture, &targetDesc, &m_targetView);
    if (FAILED(rc))
    {
        Errors::Cry(L"Couldn't create render target view. :/");
        m_texture->Release();
        return false;
    }
#endif

    // Initialize the description of the depth buffer.
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

    // Set up the description of the depth buffer.
    depthBufferDesc.Width = m_width;
    depthBufferDesc.Height = m_height;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    // Create the texture for the depth buffer using the filled out description.
    HRESULT result = device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
    if(FAILED(result))
    {
        Errors::Cry("Couldn't create depth buffer for shadow map!");
        return false;
    }

    // Initialize the depth stencil view.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // Set up the depth stencil view description.
    depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS; // D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil view.
    result = device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
    if(FAILED(result))
    {
        Errors::Cry("Couldn't create view for shadow buffer :(");
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
    resourceDesc.Format = DXGI_FORMAT_R32_FLOAT; // the formats must match or the call will fail; which raises the question, why force us to specify it multiple times then?
    resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    resourceDesc.Texture2D.MostDetailedMip = 0; // not sure about this
    resourceDesc.Texture2D.MipLevels = -1; // or this    

    rc = device->CreateShaderResourceView(m_depthStencilBuffer, &resourceDesc, &m_resourceView);
    if (FAILED(rc))
    {
        Errors::Cry(L"Couldn't create shader resource view for shadow map! :/");
        m_targetView->Release();
        m_texture->Release();
        return false;
    }

    return true;
}


void ShadowBuffer::release(void)
{
    m_depthStencilBuffer->Release();
    //m_texture->Release();

    m_depthStencilBuffer = nullptr;
    m_texture = nullptr;
}

void ShadowBuffer::setAsRenderTarget( ID3D11DeviceContext *devCtx )
{
    devCtx->OMSetRenderTargets(0, nullptr, m_depthStencilView); // this generates a bunch of warnings about a render target view being unbound but they can be safely ignored; we only want the depth buffer
}

ID3D11ShaderResourceView ** ShadowBuffer::getResourceView( ID3D11DeviceContext *devCtx )
{
    return &m_resourceView;
}

void ShadowBuffer::clear( ID3D11DeviceContext *devCtx )
{
    devCtx->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ShadowBuffer::pushToGPU( ID3D11DeviceContext *devCtx, ShadowBuffer *buffers, int numBuffers )
{
    vector<ID3D11ShaderResourceView *> views;
    views.reserve(numBuffers);

    for (int i = 0; i < numBuffers; ++i)
    {
        views.push_back(*(buffers[i].getResourceView(devCtx))); // XXX getResourceView only returns a double pointer for debugging purposes--it allows the return to be passed directly to VanillaShaderClass::Render() XXX
    }

    devCtx->PSSetShaderResources(3, numBuffers, views.data()); // collection of shadow maps starts at : register(t3)
}
