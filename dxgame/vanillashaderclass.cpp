#include "stdafx.h"
#include "vanillashaderclass.h"
#include <directxmath.h>


#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory>

using namespace std;
using namespace DirectX;

VanillaShaderClass::VanillaShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
        m_lightBuffer = 0;
        m_cameraBuffer = 0;
        ZeroMemory(m_sampleState, sizeof(m_sampleState));
}


VanillaShaderClass::VanillaShaderClass(const VanillaShaderClass& other)
{
}


VanillaShaderClass::~VanillaShaderClass()
{
}


void VanillaShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

// Render() will set vertex constant buffers and draw geometry; kind of the heart of the graphics engine?
// Vertex and index buffers must be set elsewhere before this method is called.
// 
// @deviceContext   Just get this from d3dclass.
// @indexCount      How many indexes from your index buffer do you want drawn?
// @worldMatrix     model coordinates -> world coordinates; aka Model, the M in MVP
// @viewMatrix      world -> view coordinates
// @projection      view -> screen 
// @normalMap       points to a resource view, may be nullptr
// @specularMap     points to a resource view, may be nullptr
// @lights          points to a vector of Light structures, to retrieve matrices, may be nullptr (e.g., for rendering the simple square for postprocessing)
// @texture         points to one or more texture resource views; one variant is to send 
//                  all vertexes and textures for a model at once and specify textures 
//                  at every vertex... it's even implemented but ugh? See ComplexMesh.cpp
// @numViews        the number of textures sent; defaults to 1, probably best to leave it that way
// @setSampler      leave true for normal rendering; it's set false when rendering off-screen buffer to screen,
//                  since the intermediate target class sets its own (simpler) sampler
bool VanillaShaderClass::Render( ID3D11DeviceContext *deviceContext, int indexCount, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, DirectX::CXMMATRIX projectionMatrix, ID3D11ShaderResourceView** normalMap, ID3D11ShaderResourceView** specularMap,std::vector<Light> *lights, ID3D11ShaderResourceView** texture, unsigned resourceViewCount /*= 1*/, bool setSampler /*= true*/, double animationTick /*= 1.0*/ )
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, normalMap, specularMap, lights, texture, (float)animationTick, resourceViewCount);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount, setSampler);

	return true;
}

// This loads pre-compiled shader programs and defines some data layouts
bool VanillaShaderClass::InitializeShader( ID3D11Device* device, HWND hwnd, wchar_t *vsFilename, wchar_t *psFilename )
{
    HRESULT result;
    const bool multiStreaming = false;

    struct _stat ss;

    // read the vertex shader
    _wstat(vsFilename, &ss);

    size_t vsSize = ss.st_size;
    if (!vsSize)
    {
        MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        char currentDir[1024];
        GetCurrentDirectoryA(1023, currentDir);
        cerr << "Was looking for shader in " << currentDir << endl;
        throw Errors::Fatal();
        return false;
    }

    unique_ptr<char> vertexShaderBuffer(new char [vsSize]);

    ifstream vsIn(vsFilename, ios::in | ios::binary);
    vsIn.read(vertexShaderBuffer.get(), vsSize);

    // now the pixel shader
    if (psFilename)
    {
        // we may not want a pixel shader, in case of shadow maps
        // XXX that doesn't work...
        _wstat(psFilename, &ss);

        size_t psSize = ss.st_size;
        if (!psSize)
        {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
            char currentDir[1024];
            GetCurrentDirectoryA(1023, currentDir);
            cerr << "Was looking for shader in " << currentDir << endl;
            throw Errors::Fatal();
            return false;
        }

        unique_ptr<char> pixelShaderBuffer(new char [psSize]);

        ifstream psIn(psFilename, ios::in | ios::binary);
        psIn.read(pixelShaderBuffer.get(), psSize);

        // Create the pixel shader from the buffer.
        result = device->CreatePixelShader(pixelShaderBuffer.get(), psSize, NULL, &m_pixelShader);
        if(FAILED(result))
        {
            Errors::Cry("Couldn't create shader!");
        }
    }


    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer.get(), vsSize, NULL, &m_vertexShader);
    if(FAILED(result))
    {
        Errors::Cry("Couldn't create shader!");
    }

    // Create the vertex input layout description.
    // This setup needs to match the Vertex structure
    // XXX Is this really a good place to define it then? Do we really need to redefine it for every shader if it's identical?
    D3D11_INPUT_ELEMENT_DESC polygonLayout[8];
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "NORMAL";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[1].InputSlot = multiStreaming ? 1 : 0;
    polygonLayout[1].AlignedByteOffset = multiStreaming ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    polygonLayout[2].SemanticName = "TEXCOORD";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[2].InputSlot = multiStreaming ? 2 : 0;
    polygonLayout[2].AlignedByteOffset = multiStreaming ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    polygonLayout[3].SemanticName = "TEXCOORD";
    polygonLayout[3].SemanticIndex = 1;
    polygonLayout[3].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[3].InputSlot = multiStreaming ? 3 : 0;
    polygonLayout[3].AlignedByteOffset = multiStreaming ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[3].InstanceDataStepRate = 0;

    polygonLayout[4].SemanticName = "TEXINDEX";
    polygonLayout[4].SemanticIndex = 0;
    polygonLayout[4].Format = DXGI_FORMAT_R32_UINT;
    polygonLayout[4].InputSlot = multiStreaming ? 4 : 0;
    polygonLayout[4].AlignedByteOffset = multiStreaming ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[4].InstanceDataStepRate = 0;

    polygonLayout[5].SemanticName = "TANGENT";
    polygonLayout[5].SemanticIndex = 0;
    polygonLayout[5].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[5].InputSlot = multiStreaming ? 1 : 0;
    polygonLayout[5].AlignedByteOffset = multiStreaming ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[5].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[5].InstanceDataStepRate = 0;

    polygonLayout[6].SemanticName = "BONEINDICES";
    polygonLayout[6].SemanticIndex = 0;
    polygonLayout[6].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[6].InputSlot = 0;
    polygonLayout[6].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[6].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[6].InstanceDataStepRate = 0;

    polygonLayout[7].SemanticName = "BONEWEIGHTS";
    polygonLayout[7].SemanticIndex = 0;
    polygonLayout[7].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[7].InputSlot = 0;
    polygonLayout[7].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[7].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[7].InstanceDataStepRate = 0;



    // Get a count of the elements in the layout.
    unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // Create the vertex input layout.
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer.get(), vsSize, 
		                            &m_layout);
    if(FAILED(result))
    {
	    return false;
    }

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    CD3D11_BUFFER_DESC matrixBufferDesc (sizeof(MatrixBufferType), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 0, 0);

    CD3D11_BUFFER_DESC cameraBufferDesc(matrixBufferDesc); // in the interest of brevity, copy all the identical values from a previous buffer_desc
    CD3D11_BUFFER_DESC lightBufferDesc(cameraBufferDesc);  // also these need to be copied before the first call to CreateBuffer() because it may do odd things to the data; no, really, this actually happened.
    CD3D11_BUFFER_DESC materialBufferDesc(lightBufferDesc);


    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if(FAILED(result))
    {
        Errors::Cry("Could not create constant buffer!");
	return false;
    }

    // another buffer for camera data

    cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);

    result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
    if(FAILED(result))
    {
        Errors::Cry("Could not create constant buffer!");
	return false;
    }

    // another buffer, for the light data in the pixel shader
    cameraBufferDesc.ByteWidth = sizeof(LightBufferType);
    cout << "lbf size: " << sizeof(LightBufferType) << endl;
    result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
    if(FAILED(result))
    {
        Errors::Cry("Could not create constant buffer!");
	return false;
    }

    // and a separate buffer for material properties
    materialBufferDesc.ByteWidth = sizeof(MaterialBufferType);

    result = device->CreateBuffer(&materialBufferDesc, NULL, &m_materialBuffer);
    if (FAILED(result))
    {
        Errors::Cry("Could not create material constant buffer!");
        return false;
    }

    // Create a texture sampler state description.
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 8;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state.
    result = device->CreateSamplerState(&samplerDesc, m_sampleState);
    if(FAILED(result))
    {
        Errors::Cry(L"Couldn't create sampler. Highly unlikely! :/");
	return false;
    }

    // ..._POINT would be the logical choice here but using LINEAR eliminates shadow artifacts
    // it's a non-anisotropic sampler for the copy to the swap chain and for sampling shadow maps etc.
    D3D11_SAMPLER_DESC samplerDesc2 = {D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_BORDER, 0.0f, 8, D3D11_COMPARISON_ALWAYS, {1,1,1,1}, 0, D3D11_FLOAT32_MAX };
    result = device->CreateSamplerState(&samplerDesc2, m_sampleState + 1);
    if(FAILED(result))
    {
        Errors::Cry(L"Couldn't create sampler. Highly unlikely! :/");
        return false;
    }

    // point filter with wrapping for generic use
    D3D11_SAMPLER_DESC samplerDesc3 = {D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 8, D3D11_COMPARISON_ALWAYS, {1,1,1,1}, 0, D3D11_FLOAT32_MAX };
    result = device->CreateSamplerState(&samplerDesc3, m_sampleState + 2);
    if(FAILED(result))
    {
        Errors::Cry(L"Couldn't create sampler. Highly unlikely! :/");
        return false;
    }

    // linear filter with wrapping for generic use
    D3D11_SAMPLER_DESC samplerDesc4 = {D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 8, D3D11_COMPARISON_ALWAYS, {1,1,1,1}, 0, D3D11_FLOAT32_MAX };
    result = device->CreateSamplerState(&samplerDesc4, m_sampleState + 3);
    if(FAILED(result))
    {
        Errors::Cry(L"Couldn't create sampler. Highly unlikely! :/");
        return false;
    }

    // sample+compare state for alternate shadow map treatment
    D3D11_SAMPLER_DESC samplerDesc5 = {D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_BORDER, 0.0f, 8, D3D11_COMPARISON_LESS, {1,1,1,1}, 0, D3D11_FLOAT32_MAX };
    result = device->CreateSamplerState(&samplerDesc5, m_sampleState + 4);
    if(FAILED(result))
    {
        Errors::Cry(L"Couldn't create sampler. Highly unlikely! :/");
        return false;
    }


    cout << "initialized some shaders" << endl;

    return true;
}


void VanillaShaderClass::ShutdownShader()
{
    // Release the sampler state.
    if(m_sampleState)
    {
        for (int i = 0; i < 5; ++i) if(m_sampleState[i])
        {
	    m_sampleState[i]->Release();
            m_sampleState[i] = 0;
        }
    }

    // Release the matrix constant buffer.
    if(m_matrixBuffer)
    {
	    m_matrixBuffer->Release();
	    m_matrixBuffer = 0;
    }

    // Release the layout.
    if(m_layout)
    {
	    m_layout->Release();
	    m_layout = 0;
    }

    // Release the pixel shader.
    if(m_pixelShader)
    {
	    m_pixelShader->Release();
	    m_pixelShader = 0;
    }

    // Release the vertex shader.
    if(m_vertexShader)
    {
	    m_vertexShader->Release();
	    m_vertexShader = 0;
    }

    if (m_lightBuffer)
    {
        m_lightBuffer->Release();
        m_lightBuffer = nullptr;
    }

    if (m_materialBuffer)
    {
        m_materialBuffer->Release();
        m_materialBuffer = nullptr;
    }

    return;
}


// this method mainly sets matrices and textures for the Render method; private
bool VanillaShaderClass::SetShaderParameters( ID3D11DeviceContext* deviceContext, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, DirectX::CXMMATRIX projectionMatrix, ID3D11ShaderResourceView **normalMap, ID3D11ShaderResourceView **specularMap, std::vector<Light> *lights, ID3D11ShaderResourceView **texture, float animationTick /*= 1.0f*/, unsigned numViews /*= 1*/, XMFLOAT4X4 *bindToBoneSpace )
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;

    // Lock the constant buffer so it can be written to.
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if(FAILED(result))
    {
	    return false;
    }

    // Get a pointer to the data in the constant buffer.
    dataPtr = (MatrixBufferType*)mappedResource.pData;
    ZeroMemory(dataPtr, sizeof(MatrixBufferType));
    // Copy the matrices into the constant buffer.
    XMStoreFloat4x4(&dataPtr->world, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&dataPtr->view, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&dataPtr->projection, XMMatrixTranspose(projectionMatrix));

    dataPtr->animationTick = animationTick; // animation!

    //ZeroMemory(dataPtr->lightViewProjection, sizeof(dataPtr->lightViewProjection));

    if (lights)
    {
        int j = 0;
        for (auto i = lights->begin(); i < lights->end() && i->enabled && j < NUM_SPOTLIGHTS; ++i, ++j)
        {
            XMStoreFloat4x4(&(dataPtr->lightViewProjection[j]), XMMatrixTranspose(i->getVP()));
        }
        XMStoreFloat4x4(&(dataPtr->lightViewProjection[NUM_SPOTLIGHTS]), XMMatrixTranspose((*lights)[NUM_SPOTLIGHTS].getVP()));    // directional light's shadow maps
        XMStoreFloat4x4(&(dataPtr->lightViewProjection[NUM_SPOTLIGHTS+1]), XMMatrixTranspose((*lights)[NUM_SPOTLIGHTS+1].getVP()));
        dataPtr->numLights = j;
    }

    if (bindToBoneSpace) memcpy(&dataPtr->bindToBoneSpace, bindToBoneSpace, sizeof(XMFLOAT4X4));


    // Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

    // Set the position of the constant buffer in the vertex shader.
    bufferNumber = 0;

    // Now set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    // Set shader texture resource in the pixel shader.
    deviceContext->PSSetShaderResources(0, numViews, texture);

    // same for normal map, if present
    if (normalMap && *normalMap) deviceContext->PSSetShaderResources(1, 1, normalMap);

    // and specular map
    if (specularMap && *specularMap) deviceContext->PSSetShaderResources(2, 1, specularMap);


    return true;
}

// set material values; these are conventional parameters
bool VanillaShaderClass::SetPSMaterial( ID3D11DeviceContext *deviceContext, DirectX::XMFLOAT4 &ambientColor, DirectX::XMFLOAT4 &diffuseColor, float specularPower, DirectX::XMFLOAT4 &specularColor, bool useNormalMap, bool useSpecularMap )
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MaterialBufferType* dataPtr;

    //deviceContext->VSSetShader(m_vertexShader, NULL, 0); // TODO change this to an "effect?" or are effects going out of style?
    //deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    result = deviceContext->Map(m_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    dataPtr = (MaterialBufferType*)mappedResource.pData;
    ZeroMemory(dataPtr, sizeof(MaterialBufferType));
    dataPtr->ambientColor = ambientColor;
    dataPtr->diffuseColor = diffuseColor;
    dataPtr->specularColor = specularColor;
    dataPtr->specularPower = specularPower;
    dataPtr->useNormalMap = useNormalMap;
    dataPtr->useSpecularMap = useSpecularMap;
    dataPtr->padding = 0;

    deviceContext->Unmap(m_materialBuffer, 0);

    deviceContext->PSSetConstantBuffers(0, 1, &m_materialBuffer);

    return true;
}

// This method writes lighting data into a pixel shader constant buffer.
// lightDirection is for the solitary directional light (e.g., sunlight)
// time is the output Chronometer::sinceInit(), for time-dependent special effects
// cameraPos is the location of the eye in worldspace; comes from FirstPerson
// spotlightPos is an array of spotlight positions
// spotlightDir is an array of spotlight directions
// spotlightParams is an array mapping to spotlightEtc in the constant buffer
//      x = Cos(beamAngle), y = constant attenuation, z = linear attenuation, w = quadrating attenuation
// numSpotlights is the number of elements in the arrays above; valid range is [0..NUM_SPOTLIGHTS) 
//      NOTE Perhaps the arrays should be vector instead?
bool VanillaShaderClass::SetPSLights( ID3D11DeviceContext *deviceContext, const DirectX::FXMVECTOR lightDirection, float time, const DirectX::FXMVECTOR cameraPos, std::vector<Light> &lights, 
                                     const DirectX::XMFLOAT4 &ambientLight, const DirectX::XMFLOAT4 &diffuseLight )
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    LightBufferType* dataPtr;

    //deviceContext->VSSetShader(m_vertexShader, NULL, 0); // TODO change this to an "effect?" or are effects going out of style?
    //deviceContext->PSSetShader(m_pixelShader, NULL, 0);  // XXX Will this be a hit on performance? Perhaps make a separate function to set these once?


    result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    dataPtr = (LightBufferType*)mappedResource.pData;

    XMStoreFloat3(&dataPtr->lightDirection, lightDirection);
    dataPtr->time = time;
    XMStoreFloat4(&dataPtr->cameraPos, cameraPos);
    
    int j = 0, numLights = 0;
    for (auto i = lights.begin(); i != lights.end() && j < NUM_SPOTLIGHTS; ++i, ++j)
    {
        if (i->enabled)
        {
            numLights = j+1;
            dataPtr->spotlightPos[j] = i->position;
            dataPtr->spotlightDir[j] = XMFLOAT4(i->direction.x, i->direction.y, i->direction.z, 0);
            dataPtr->spotlightEtc[j].x = i->cosHalfAngle;
            dataPtr->spotlightEtc[j].y = i->constantAttenuation;
            dataPtr->spotlightEtc[j].z = i->linearAttenuation;
            dataPtr->spotlightEtc[j].w = i->quadraticAttenuation;
            dataPtr->spotlightCol[j] = i->color;
        } else
        {
            dataPtr->spotlightPos[j] = XMFLOAT4(0,0,0,0);
            dataPtr->spotlightDir[j] = XMFLOAT4(0,0,0,0);
            dataPtr->spotlightEtc[j] = XMFLOAT4(0,0,0,0);
            dataPtr->spotlightCol[j] = XMFLOAT4(0,0,0,0);
        }

    }

    dataPtr->ambientLight = ambientLight;
    dataPtr->diffuseLight = diffuseLight; // from directional light, that is

    dataPtr->numLights = numLights;

    deviceContext->Unmap(m_lightBuffer, 0);

    deviceContext->PSSetConstantBuffers(1, 1, &m_lightBuffer); // light buffer is register(cb1) aka slot one

    return true;

}


void VanillaShaderClass::RenderShader( ID3D11DeviceContext *deviceContext, int indexCount, bool setSampler /*= true*/ )
{
    // Set the vertex input layout.
    deviceContext->IASetInputLayout(m_layout);

    deviceContext->VSSetShader(m_vertexShader, NULL, 0); // TODO change this to an "effect?" or are effects going out of style?
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // Set the sampler state in the pixel shader.
    if (setSampler) deviceContext->PSSetSamplers(0, 5, m_sampleState);

    // Render the triangles
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}

bool VanillaShaderClass::setVSCameraBuffer( ID3D11DeviceContext* deviceContext, DirectX::CXMVECTOR cameraPos, float time, unsigned effect )
{
    HRESULT result;
    CameraBufferType *cDataPtr;
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    //
    // camera constant buffer!
    //
    result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if(FAILED(result))
    {
        return false;
    }

    // Get a pointer to the data in the constant buffer.
    cDataPtr = (CameraBufferType*)mappedResource.pData;

    XMStoreFloat4(&cDataPtr->cameraPosition, cameraPos);
    cDataPtr->time = time;
    cDataPtr->effect = effect;

    deviceContext->Unmap(m_cameraBuffer, 0);

    deviceContext->VSSetConstantBuffers(1, 1, &m_cameraBuffer); // camera buffer is second in shader

    return true;
}
