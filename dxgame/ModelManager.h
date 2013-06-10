#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include "CompoundMesh.h"
#include "TextureManager.h"

#include <vector>
#include <string>
#include <unordered_map>
#include "d3dclass.h"

class ModelManager
{
private:
    std::vector<CompoundMesh> modelLibrary;
    std::unordered_map<std::string, int> modelReference;

    ID3D11DeviceContext *m_deviceContext; // caching these here because this is a high level class dammit and it'll shorten the parameter lists
    ID3D11Device *m_device;
    TextureManager m_texMgr;

public:
    ModelManager(D3DClass &d3d);
    ModelManager(ModelManager &);
    ModelManager &operator=(ModelManager &);
    ~ModelManager(void);

    int getRefNum(std::string name);
    bool alias(std::string from, std::string to);
    
    // Warning! Pointers retrieved through these methods may be invalid after a load operation. 
    // Load all your stuff in one pass or else look up everything every frame
    // alternately, storing refnums is safe
    CompoundMesh *operator [] (string name);
    CompoundMesh *operator [] (int refNum);

    TextureManager *getTextureManager() { return &m_texMgr; }
    int getLibrarySize() { return modelLibrary.size(); }
    // std::vector<CompoundMesh> &getLibrary() { return modelLibrary; } // meh?
};

#endif