#include "stdafx.h"

#include "LoadedTexture.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

#include <memory>



LoadedTexture::LoadedTexture() : m_texture(nullptr), m_resource(nullptr)
{
}


LoadedTexture::LoadedTexture(const LoadedTexture& other)
{
    *this = other;
}


LoadedTexture::~LoadedTexture()
{
}


bool LoadedTexture::initialize(ID3D11Device* device, ID3D11DeviceContext* ctx, wchar_t* filename)
{
	HRESULT result;


	// Load the texture in.
	// result = D3DX11CreateShaderResourceViewFromFile(device, filename, NULL, NULL, &m_texture, NULL); // obsolete :[

        if (wcsstr(filename, L".dds") || wcsstr(filename, L".DDS"))
        {
            result = DirectX::CreateDDSTextureFromFile(device, filename, &m_resource, &m_texture);
            if (FAILED(result))
            {
                return false;
            } else
                return true;
        }
	
	result = DirectX::CreateWICTextureFromFile(device, ctx, filename, nullptr, &m_texture);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


void LoadedTexture::shutdown()
{
	// Release the texture resource.
	if(m_texture)
	{
	    m_texture->Release();
	    m_texture = nullptr;
	}

        if (m_resource)
        {
            m_resource->Release();
            m_resource = nullptr;
        }

	return;
}


ID3D11ShaderResourceView **LoadedTexture::getTexture()
{
    if (m_texture)
    {
	return &m_texture;
    } else
    {
        return nullptr;
    }
}