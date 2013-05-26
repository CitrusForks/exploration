#include "StdAfx.h"

#include "TextureManager.h"
#include "Errors.h"
#include "windows.h"

using namespace std;

TextureManager::TextureManager(void)
{
}

TextureManager::TextureManager( TextureManager &no )
{
    Errors::Cry("You can't copy TextureManager! Use a pointer! >:| Everything will break now.");
}

TextureManager & TextureManager::operator=( TextureManager &nope )
{
    Errors::Cry("You can't copy TextureManager with operator=() either! You've doomed us all!");
    return *this;
}



TextureManager::~TextureManager(void)
{
    for (auto i = m_textureReference.begin(); i != m_textureReference.end(); ++i) (*i).second.shutdown();
    // writing the above loop with for_each() makes it more verbose and less readable :/

    m_textureReference.clear();
}

bool TextureManager::getTextureUTF8( ID3D11Device *device, ID3D11DeviceContext *devCtx, char *c_path, int c_len, LoadedTexture &out )
{
    while (*c_path == '\\' || *c_path == '/' || *c_path == '.') { ++c_path; --c_len; } // cut off any path stuff
    wstring path = utf8ToWstring(c_path, c_len);


    return getTexture(path, device, devCtx, out);

}

bool TextureManager::getTexture( wstring path, ID3D11Device * device, ID3D11DeviceContext * devCtx, LoadedTexture &out )
{
    auto reference = m_textureReference.find(path);  // have we loaded it already?

    if (reference == m_textureReference.end())
    {
        // texture not found, load it
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
        std::string u8path = conv1.to_bytes(path.c_str());

        cout << "Loading new texture: " << u8path << endl;

        if (!out.initialize(device, devCtx, (wchar_t*)path.c_str()))
        {
            Errors::Cry("Failed to load texture, ", (char*)u8path.c_str());
            return false;
        } else
        {
            m_textureReference[path] = out;
            return true; // success!
        }
    } else
    {
        out = reference->second; // already loaded! 
        return true;
    }
}

// XXX put this somewhere else.
wstring TextureManager::utf8ToWstring( char * c_path, int c_len )
{
    int length = MultiByteToWideChar(CP_UTF8, 0, c_path, c_len + 1, 0, 0);  // get length of wchar_t result
    unique_ptr<wchar_t>wPath(new wchar_t[length]);  // allocate buffer
    MultiByteToWideChar(CP_UTF8, 0, c_path, c_len + 1, wPath.get(), length); // get the path in wchar_t
    wstring path(wPath.get());  // and wstring

    return path;
}

