////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "textureclass.h"

#include <memory>



TextureClass::TextureClass()
{
	m_texture = 0;
}


TextureClass::TextureClass(const TextureClass& other)
{
}


TextureClass::~TextureClass()
{
}


bool TextureClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* ctx, wchar_t* filename)
{
	HRESULT result;


	// Load the texture in.
	// result = D3DX11CreateShaderResourceViewFromFile(device, filename, NULL, NULL, &m_texture, NULL); // obsolete :[
	
	result = DirectX::CreateWICTextureFromFile(device, ctx, filename, nullptr, &m_texture);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


void TextureClass::Shutdown()
{
	// Release the texture resource.
	if(m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}

	return;
}


ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_texture;
}