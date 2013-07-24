#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include "LoadedTexture.h"
#include <string>

class TextureManager
{
private:
    std::unordered_map<wstring, LoadedTexture> m_textureReference;

public:
    bool TextureManager::getTextureUTF8(ID3D11Device *device, ID3D11DeviceContext *devCtx, char *c_path, int c_len, LoadedTexture &out);

    bool getTexture( wstring path, ID3D11Device * device, ID3D11DeviceContext * devCtx, LoadedTexture &out );

    // utility functions that probably belong elsewhere (TODO):
    std::wstring utf8ToWstring( char * c_path, int c_len );
    void dePath( wstring &path );


    TextureManager(void);
    ~TextureManager(void);
    TextureManager(TextureManager &no);
    TextureManager &operator=(TextureManager &nope);
};

#endif