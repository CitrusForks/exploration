#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_


#include <d3d11.h>
#include <d3dx11tex.h>


class LoadedTexture
{
public:
	LoadedTexture();
	LoadedTexture(const LoadedTexture&);
	~LoadedTexture();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, wchar_t*);
	void Shutdown();

	ID3D11ShaderResourceView** GetTexture();
private:
	ID3D11ShaderResourceView* m_texture;
};

#endif