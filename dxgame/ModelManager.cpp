#include "StdAfx.h"
#include "ModelManager.h"
#include "Errors.h"

#include <stdlib.h>
#include <unordered_map>

using namespace std;

// disabled copy constructor and assignment
ModelManager::ModelManager(ModelManager &no)
{
    Errors::Cry("You can't copy ModelManager!");
    exit(1);
}

ModelManager::ModelManager( ID3D11Device *device, ID3D11DeviceContext *devCtx ) : m_device(device), m_deviceContext(devCtx)
{

}


ModelManager &ModelManager::operator=(ModelManager &nope)
{
    Errors::Cry("You can't assign ModelManager!");
    exit(1);
}


ModelManager::~ModelManager(void)
{
    for (auto i = modelLibrary.begin(); i != modelLibrary.end(); ++i)
    {
        i->release();
    }

    modelLibrary.clear();
}

int ModelManager::getRefNum( std::string name )
{
    auto ref = modelReference.find(name);

    if (ref != modelReference.cend())
    {
        return ref->second;
    } else
    {
        CompoundMesh newModel;
        if (!newModel.load(m_device, m_deviceContext, &m_texMgr, (char*)name.c_str()))
        {
            Errors::Cry("Could not load model, ", (char*)name.c_str());
            return -1; // this is an error code, take care
        }

        modelReference[name] = modelLibrary.size();
        modelLibrary.push_back(newModel);

        return modelReference[name];
    }    
}

bool ModelManager::alias( std::string from, std::string to )
{
    int ref = getRefNum(from);

    if (ref == -1) return false;

    modelReference[to] = ref;

    return true;
}


CompoundMesh * ModelManager::operator[]( int refNum )
{
    assert(refNum != -1);
    assert(refNum < (int)modelLibrary.size());

    CompoundMesh *ret = &(modelLibrary.data()[refNum]);
    return ret;
}


CompoundMesh * ModelManager::operator[]( string name )
{
    int ref = getRefNum(name);

    if (ref == -1) return nullptr; // take heed!

    return (*this)[ref];
}

