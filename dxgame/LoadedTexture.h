#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_


#include <d3d11.h>
#include <d3dx11tex.h>

// this is little more than a light weight wrapper for ID3D11ShaderResourceView*
// as such, it's best to store and copy this object directly
class LoadedTexture
{
public:
	LoadedTexture();
	LoadedTexture(const LoadedTexture&);
	~LoadedTexture();

	bool initialize(ID3D11Device*, ID3D11DeviceContext*, wchar_t*);
	void shutdown();

	ID3D11ShaderResourceView** getTexture();
private:
	ID3D11ShaderResourceView* m_texture;
};

#endif