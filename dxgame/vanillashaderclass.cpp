////////////////////////////////////////////////////////////////////////////////
// Filename: textureshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "vanillashaderclass.h"
#include <directxmath.h>
#include <d3dcompiler.h>


#include <iostream>

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
	m_sampleState[0] = 0;
        m_sampleState[1] = 0;
        m_sampleState[2] = 0;
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
// @texture         points to one or more texture resource views; one variant is to send 
//                  all vertexes and textures for a model at once and specify textures 
//                  at every vertex... it's even implemented but ugh? See ComplexMesh.cpp
// @numViews        the number of textures sent; defaults to 1, probably best to leave it that way
// @setSampler      leave true for normal rendering; it's set false when rendering off-screen buffer to screen,
//                  since the intermediate target class sets its own (simpler) sampler
bool VanillaShaderClass::Render( ID3D11DeviceContext *deviceContext, int indexCount, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, DirectX::CXMMATRIX projectionMatrix, ID3D11ShaderResourceView** normalMap, ID3D11ShaderResourceView** specularMap,DirectX::XMFLOAT4X4 *lightProjections, int numShadows, ID3D11ShaderResourceView** texture, unsigned resourceViewCount /*= 1*/, bool setSampler /*= true*/ )
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, normalMap, specularMap, lightProjections, numShadows, texture, resourceViewCount);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount, setSampler);

	return true;
}

// This method actually compiles shader programs, via the compiler dll. N.B., that's not allowed in the Windows App Store but is that a thing that's actually relevant to anyone's life?
bool VanillaShaderClass::InitializeShader( ID3D11Device* device, HWND hwnd, wchar_t *vsFilename, wchar_t *psFilename, bool multiStreaming /*= false*/ )
{
    HRESULT result;
    ID3D10Blob* errorMessage = 0;

    ID3D10Blob* vertexShaderBuffer = nullptr;

    // Old code to compile shader at runtime:
    //result = D3DX11CompileFromFile(vsFilename, NULL, NULL, vsFunctionName, "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								//&vertexShaderBuffer, &errorMessage, NULL); // XXX D3DX11 is deprecated, replace this with CompileFromFile()

    //result = D3DCompileFromFile(vsFilename, nullptr, nullptr, vsFunctionName, "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);

	// actually, how about load bytecode...
	result = D3DReadFileToBlob(vsFilename, &vertexShaderBuffer);
    if(FAILED(result))
    {
	    // If the shader failed to compile it should have writen something to the error message.
	    if(errorMessage)
	    {
		    OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
	    }
	    // If there was nothing in the error message then it simply could not find the shader file itself.
	    else
	    {
		    MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
	    }

	    return false;
    }


    ID3D10Blob* pixelShaderBuffer = nullptr;

    //result = D3DCompileFromFile(psFilename, NULL, NULL, psFunctionName, "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage); 
	result = D3DReadFileToBlob(psFilename, &pixelShaderBuffer);
    if(FAILED(result))
    {
	    // If the shader failed to compile it should have writen something to the error message.
	    if(errorMessage)
	    {
		    OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
	    }
	    // If there was  nothing in the error message then it simply could not find the file itself.
	    else
	    {
		    MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
	    }

	    return false;
    }

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if(FAILED(result))
    {
	    return false;
    }

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if(FAILED(result))
    {
	    return false;
    }

    // Create the vertex input layout description.
    // This setup needs to match the Vertex structure
    // XXX Is this really a good place to define it then?
    D3D11_INPUT_ELEMENT_DESC polygonLayout[6];
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


    // Get a count of the elements in the layout.
    unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // Create the vertex input layout.
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), 
		                            &m_layout);
    if(FAILED(result))
    {
	    return false;
    }

    // Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    D3D11_BUFFER_DESC cameraBufferDesc = matrixBufferDesc; // in the interest of brevity, copy all the identical values from a previous buffer_desc
    D3D11_BUFFER_DESC lightBufferDesc = cameraBufferDesc;  // also these need to be copied before the first call to CreateBuffer() because it may do odd things to the data; no, really, this actually happened.
    D3D11_BUFFER_DESC materialBufferDesc = lightBufferDesc;


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
    //samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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
	    return false;
    }

    // ..._POINT would be the logical choice here but using LINEAR eliminates shadow artifacts
    // it's a non-anisotropic sampler for the copy to the swap chain and for sampling shadow maps etc.
    D3D11_SAMPLER_DESC samplerDesc2 = {D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_BORDER, 0.0f, 8, D3D11_COMPARISON_ALWAYS, {1,1,1,1}, 0, D3D11_FLOAT32_MAX };

    result = device->CreateSamplerState(&samplerDesc2, m_sampleState + 1);
    if(FAILED(result))
    {
        Errors::Cry(L"Couldn't create sampler for intermediate buffer. :/");
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
	    m_sampleState[0]->Release();
	    m_sampleState[0] = 0;
            m_sampleState[1]->Release();
            m_sampleState[1] = 0;
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


void VanillaShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	cerr << "Error compiling shader:";

	// Write out the error message.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
		cerr << compileErrors[i];
	}

        cerr << endl;

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

// this method mainly sets matrices and textures for the Render method; private
bool VanillaShaderClass::SetShaderParameters( ID3D11DeviceContext* deviceContext, DirectX::CXMMATRIX worldMatrix, DirectX::CXMMATRIX viewMatrix, DirectX::CXMMATRIX projectionMatrix, ID3D11ShaderResourceView **normalMap, ID3D11ShaderResourceView **specularMap, DirectX::XMFLOAT4X4* lightProjections, unsigned numLights, ID3D11ShaderResourceView **texture, unsigned numViews /*= 1*/ )
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;
    //XMFLOAT4X4 worldF4X4, viewF4X4, projectionF4X4;    

    // Transpose the matrices to prepare them for the shader.

    //D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
    //D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
    //D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

    deviceContext->VSSetShader(m_vertexShader, NULL, 0); // TODO change this to an "effect?" or are effects going out of style?
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);


    // Lock the constant buffer so it can be written to.
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if(FAILED(result))
    {
	    return false;
    }

    // Get a pointer to the data in the constant buffer.
    dataPtr = (MatrixBufferType*)mappedResource.pData;

    // Copy the matrices into the constant buffer.
    XMStoreFloat4x4(&dataPtr->world, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&dataPtr->view, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&dataPtr->projection, XMMatrixTranspose(projectionMatrix));

    ZeroMemory(dataPtr->lightViewProjection, sizeof(dataPtr->lightViewProjection));

    for (unsigned i = 0; i < numLights; ++i)
    {
        XMStoreFloat4x4(&(dataPtr->lightViewProjection[i]), XMMatrixTranspose(XMLoadFloat4x4(lightProjections+i)));
    }
    XMStoreFloat4x4(&(dataPtr->lightViewProjection[NUM_SPOTLIGHTS]), XMMatrixTranspose(XMLoadFloat4x4(lightProjections+NUM_SPOTLIGHTS)));    // directional light's shadow maps
    XMStoreFloat4x4(&(dataPtr->lightViewProjection[NUM_SPOTLIGHTS+1]), XMMatrixTranspose(XMLoadFloat4x4(lightProjections+NUM_SPOTLIGHTS+1)));
    dataPtr->numLights = numLights;

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

    deviceContext->VSSetShader(m_vertexShader, NULL, 0); // TODO change this to an "effect?" or are effects going out of style?
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    result = deviceContext->Map(m_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    dataPtr = (MaterialBufferType*)mappedResource.pData;

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
bool VanillaShaderClass::SetPSLights( ID3D11DeviceContext *deviceContext, const DirectX::XMFLOAT3 &lightDirection, float time, DirectX::FXMVECTOR cameraPos, DirectX::XMFLOAT4 *spotlightPos, DirectX::XMFLOAT3 *spotlightDir, DirectX::XMFLOAT4 *spotlightParams, int numSpotlights, DirectX::XMFLOAT4 &ambientLight, DirectX::XMFLOAT4 &diffuseLight )
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    LightBufferType* dataPtr;

    deviceContext->VSSetShader(m_vertexShader, NULL, 0); // TODO change this to an "effect?" or are effects going out of style?
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);  // XXX Will this be a hit on performance? Perhaps make a separate function to set these once?


    result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    dataPtr = (LightBufferType*)mappedResource.pData;

    dataPtr->lightDirection = lightDirection;
    dataPtr->time = time;
    XMStoreFloat4(&dataPtr->cameraPos, cameraPos);
    
    for (int i = 0; i < numSpotlights; ++i)
    {
        dataPtr->spotlightPos[i] = spotlightPos[i];
        dataPtr->spotlightDir[i] = XMFLOAT4(spotlightDir[i].x, spotlightDir[i].y, spotlightDir[i].z, 0);
        if (!spotlightParams)
        {
            dataPtr->spotlightEtc[i].x = cosf((float)M_PI * 15.0f/180.f); // angle from axis to lateral surface (or "generatrix" if you want to look up some terms for cone parts)
            dataPtr->spotlightEtc[i].y = 0.75f; // constant attenuation 
            dataPtr->spotlightEtc[i].z = 0.0f; // linear
            dataPtr->spotlightEtc[i].w = 0.01f; // d^2 
        } else
        {
            dataPtr->spotlightEtc[i] = spotlightParams[i];
        }
    }

    for (int i = numSpotlights; i < NUM_SPOTLIGHTS; ++i)
    {
        dataPtr->spotlightPos[i] = XMFLOAT4(0,0,0,0);
        dataPtr->spotlightDir[i] = XMFLOAT4(0,0,0,0);
        dataPtr->spotlightEtc[i] = XMFLOAT4(0,0,0,0);
    }

    dataPtr->ambientLight = ambientLight;
    dataPtr->diffuseLight = diffuseLight; // from directional light, that is

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
    if (setSampler) deviceContext->PSSetSamplers(0, 2, m_sampleState);

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
