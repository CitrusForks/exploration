#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include "LoadedTexture.h"
#include <string>

class TextureManager
{
private:
    std::unordered_map<wstring, LoadedTexture> m_textureReference;

public:
    LoadedTexture TextureManager::getTextureUTF8(ID3D11Device *device, ID3D11DeviceContext *devCtx, char *c_path, int c_len);

    std::wstring utf8ToWstring( char * c_path, int c_len );

    LoadedTexture getTexture( wstring path, ID3D11Device * device, ID3D11DeviceContext * devCtx );

    TextureManager(void);
    ~TextureManager(void);
    TextureManager(TextureManager &no);
    TextureManager &operator=(TextureManager &nope);
};

#endif